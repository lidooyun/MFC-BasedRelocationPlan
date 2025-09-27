import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.distance import cdist

# 设置中文显示
plt.rcParams['font.sans-serif'] = ['SimHei']  # 设置为黑体 SimHei
plt.rcParams['axes.unicode_minus'] = False    # 解决负号显示为方块问题

def load_data():
    # 尝试用不同编码加载数据
    try:
        # 加载院落离街道的距离.csv
        distance_df = pd.read_csv('院落离街道的距离.csv', encoding='gbk')
        # 加载附件一：老城街区地块信息.csv
        plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gbk')
        print("成功使用GBK编码读取文件")
    except:
        try:
            # 如果GBK失败，尝试使用UTF-8
            distance_df = pd.read_csv('院落离街道的距离.csv', encoding='utf-8')
            plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='utf-8')
            print("成功使用UTF-8编码读取文件")
        except:
            try:
                # 如果UTF-8失败，尝试使用GB2312
                distance_df = pd.read_csv('院落离街道的距离.csv', encoding='gb2312')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gb2312')
                print("成功使用GB2312编码读取文件")
            except:
                # 如果GB2312也失败，尝试指定ANSI
                distance_df = pd.read_csv('院落离街道的距离.csv', encoding='ansi')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='ansi')
                print("成功使用ANSI编码读取文件")

    # 检查数据
    print("\n院落离街道距离数据预览：")
    print(distance_df.head())
    print("\n地块信息数据预览：")
    print(plots_df.head())

    # 确保列名符合后续处理需求
    if '地块ID' in plots_df.columns:
        plots_df.columns = ['地块编号', '院落编号', '地块面积', '院落面积', '方位', '是否有人居住']
    
    return distance_df, plots_df

def calculate_courtyard_density(distance_df):
    """计算院落周边密度并输出到文件"""
    # 计算距离的估计中点r
    r = (distance_df['distance'].max() + distance_df['distance'].min()) / 2
    print(f"\n距离估计中点r: {r}")

    # 计算院落之间的欧几里得距离
    courtyard_coords = distance_df[['x', 'y']].values
    distances = cdist(courtyard_coords, courtyard_coords, 'euclidean')

    # 计算每个院落的周边密度(半径r圆内有几个院落)
    densities = []
    for i in range(len(distance_df)):
        # 计算距离小于r的院落数量(不包括自己)
        density = np.sum(distances[i] < r) - 1  # 减1是因为自己到自己的距离为0
        densities.append(density)

    # 添加密度列到数据中
    distance_df['周边密度'] = densities

    # 计算理想密度(平均密度)
    ideal_density = np.mean(densities)
    print(f"理想密度: {ideal_density}")

    # 将结果写入院落密度.csv
    density_output = distance_df[['id', 'x', 'y', '周边密度']]
    density_output.to_csv('院落密度.csv', index=False, encoding='gbk')
    print("院落密度数据已写入'院落密度.csv'")

    return distance_df, ideal_density

def calculate_public_space(plots_df):
    """计算每个院落的公共区域面积和每个地块的平均公共区域面积"""
    # 按院落编号分组,计算每个院落包含的地块面积总和
    courtyard_plot_areas = plots_df.groupby('院落编号')['地块面积'].sum().reset_index()
    courtyard_plot_areas.columns = ['院落编号', '地块面积总和']

    # 获取每个院落的面积(从任意一个对应地块记录中提取)
    courtyard_areas = plots_df[['院落编号', '院落面积']].drop_duplicates()

    # 合并数据
    courtyard_data = pd.merge(courtyard_areas, courtyard_plot_areas, on='院落编号')

    # 计算公共区域面积 = 院落面积 - 地块面积总和 (如果为负则取0)
    courtyard_data['公共区域面积'] = courtyard_data.apply(
        lambda row: max(0, row['院落面积'] - row['地块面积总和']),
        axis=1)

    # 计算每个院落的地块数量
    plot_count = plots_df.groupby('院落编号').size().reset_index(name='地块数量')
    courtyard_data = pd.merge(courtyard_data, plot_count, on='院落编号')

    # 计算平均公共区域面积
    courtyard_data['平均公共区域面积'] = courtyard_data['公共区域面积'] / courtyard_data['地块数量']

    print("\n公共区域面积计算结果预览:")
    print(courtyard_data.head())

    return courtyard_data

def prepare_relocation_data(plots_df, distance_df, courtyard_data, ideal_density):
    """准备搬迁评估所需的数据"""
    # 方位对应的采光效果字典
    light_effect = {
        '南': 1,
        '北': 1,
        '东': 0.8,
        '西': 0.6
    }

    # 将方位转换为采光效果
    plots_df['采光效果'] = plots_df['方位'].map(light_effect)

    # 合并院落距离和密度数据到地块数据
    plots_with_distance = pd.merge(
        plots_df,
        distance_df[['id', 'distance', '周边密度']],
        left_on='院落编号',
        right_on='id',
        how='left'
    ).drop(columns=['id'])

    # 合并平均公共区域面积数据
    final_df = pd.merge(
        plots_with_distance,
        courtyard_data[['院落编号', '平均公共区域面积']],
        on='院落编号',
        how='left'
    )

    print("\n合并后的数据预览:")
    print(final_df.head())

    return final_df, ideal_density

def calculate_scores(occupied_plots, empty_plots, ideal_density):
    """计算每户居民对每个可能搬迁地块的评分"""
    # 计算评分的常量
    distance_max = occupied_plots['distance'].max()
    distance_min = occupied_plots['distance'].min()
    density_max = occupied_plots['周边密度'].max()
    density_min = occupied_plots['周边密度'].min()
    space_max = occupied_plots['平均公共区域面积'].max()
    space_min = occupied_plots['平均公共区域面积'].min()

    # 确保常量计算正确（避免除以零的情况）
    if distance_max == distance_min:
        distance_max += 0.01
    if density_max == density_min:
        density_max += 0.01
    if space_max == space_min:
        space_max += 0.01

    # 修改后的评分函数定义
    def Uarea(area_i, area_j):
        """面积评分 - 修改后为 A_j/A_i - 1"""
        return (10/3) * (area_j / area_i - 1)

    def Usun(sun_i, sun_j):
        """采光评分 - 修改为 2.5 - 2.5*(sun_i / sun_j)"""
        return 2.5 - 2.5 * (sun_i / sun_j)

    def Uroad(distance_i, distance_j):
        """街道距离评分"""
        if distance_i - distance_j > 0:
            return (distance_i - distance_j) / (distance_max - distance_min)
        return 0

    def Udens(density_i, density_j):
        """密度评分 - 处理负数和大于1的情况"""
        score = (3/2) * abs((density_i - ideal_density) / (density_max - density_min)) - \
                (1/2) * abs((density_j - ideal_density) / (density_max - density_min))
        if score < 0:
            return 0
        elif score > 1:
            return 1
        else:
            return score

    def Uspace(space_i, space_j):
        """公共空间评分"""
        if space_i - space_j > 0:
            return (space_i - space_j) / (space_max - space_min)
        return 0

    def Utotal(scores, weights=[0.5939, 0.2115, 0.0753, 0.0524, 0.0669]):
        """总评分"""
        return sum(score * weight for score, weight in zip(scores, weights))

    # 计算每户居民对每个可能搬迁地块的评分
    results = []

    # 首先添加不搬迁的情况
    for _, resident in occupied_plots.iterrows():
        # 不搬迁的情况（自己搬到自己的地块）
        results.append({
            '居民地块编号': resident['地块编号'],
            '居民院落编号': resident['院落编号'],
            '目标地块编号': resident['地块编号'],
            '目标院落编号': resident['院落编号'],
            '居民地块面积': resident['地块面积'],
            '目标地块面积': resident['地块面积'],
            '原院落面积': resident['院落面积'],
            '新院落面积': resident['院落面积'],
            '原地块方位': resident['方位'],
            '新地块方位': resident['方位'],
            '面积评分': 1,
            '采光评分': 1,
            '街道距离评分': 1,
            '密度评分': 1,
            '公共空间评分': 1,
            '总评分': 1,
            '评分差值': 0,
            'repair': 0,
            '是否搬迁': 0  # 标记为不搬迁
        })

    # 然后添加搬迁的情况
    for _, resident in occupied_plots.iterrows():
        for _, empty_plot in empty_plots.iterrows():
            # 剪枝条件: 不考虑面积更小的地块、面积超过1.3倍的地块、采光效果更差的地块
            if (empty_plot['地块面积'] < resident['地块面积'] or
                empty_plot['地块面积'] > resident['地块面积'] * 1.3 or
                empty_plot['采光效果'] < resident['采光效果']):
                continue

            # 计算各项评分
            area_score = Uarea(resident['地块面积'], empty_plot['地块面积'])
            sun_score = Usun(resident['采光效果'], empty_plot['采光效果'])
            road_score = Uroad(resident['distance'], empty_plot['distance'])
            dens_score = Udens(resident['周边密度'], empty_plot['周边密度'])
            space_score = Uspace(resident['平均公共区域面积'], empty_plot['平均公共区域面积'])

            # 总评分
            scores = [area_score, sun_score, road_score, dens_score, space_score]
            total_score = Utotal(scores)

            # 添加到结果，增加院落面积和方位信息
            results.append({
                '居民地块编号': resident['地块编号'],
                '居民院落编号': resident['院落编号'],
                '目标地块编号': empty_plot['地块编号'],
                '目标院落编号': empty_plot['院落编号'],
                '居民地块面积': resident['地块面积'],
                '目标地块面积': empty_plot['地块面积'],
                '原院落面积': resident['院落面积'],
                '新院落面积': empty_plot['院落面积'],
                '原地块方位': resident['方位'],
                '新地块方位': empty_plot['方位'],
                '面积评分': area_score,
                '采光评分': sun_score,
                '街道距离评分': road_score,
                '密度评分': dens_score,
                '公共空间评分': space_score,
                '总评分': total_score,
                '是否搬迁': 1  # 标记为搬迁
            })

    # 创建结果DataFrame并保存
    results_df = pd.DataFrame(results)
    results_df.to_csv('初步搬迁意向.csv', index=False, encoding='gbk')
    print("\n初步搬迁意向数据已写入'初步搬迁意向.csv'")

    return results_df

def calculate_repair_and_filter(results_df, plots_df):
    """计算repair并筛选符合条件的搬迁方案"""
    # 使用75%分位数而非平均值作为目标评分
    target_score = results_df[results_df['是否搬迁'] == 1]['总评分'].quantile(0.5)
    #target_score = 0.95
    print(f"\n目标评分 (75%分位数): {target_score}")

    # 计算所有地块面积中的最小值
    min_plot_area = plots_df['地块面积'].min()
    print(f"最小地块面积: {min_plot_area}")

    # 计算每个搬迁方案的repair（对于不搬迁的情况，评分差值和repair已在之前设置为0）
    results_df['目标评分'] = target_score

    # 只对是否搬迁为1且目标评分比当前总评分大的方案计算差值
    results_df['评分差值'] = results_df.apply(
        lambda row: max(0, row['目标评分'] - row['总评分']) if row['是否搬迁'] == 1 else 0,
        axis=1)

    # 计算repair
    results_df['repair'] = results_df.apply(
        lambda row: row['评分差值'] * row['目标地块面积'] * 200000 / min_plot_area if row['是否搬迁'] == 1 else 0,
        axis=1)

    # 限制repair不能超过200000（对于搬迁情况）
    valid_relocations = results_df[(results_df['是否搬迁'] == 0) | 
                                   ((results_df['是否搬迁'] == 1) & (results_df['repair'] <= 200000))].copy()

    # 将结果写入每户意愿.csv
    # 修改部分：调整列的顺序，把目标评分放在总评分之后，并且不输出是否搬迁列
    columns_order = [col for col in valid_relocations.columns if col != '是否搬迁']
    # 确保目标评分在总评分后面
    total_score_index = columns_order.index('总评分')
    target_score_index = columns_order.index('目标评分')
    columns_order.remove('目标评分')
    columns_order.insert(total_score_index + 1, '目标评分')
    
    # 使用调整后的列顺序保存
    valid_relocations[columns_order].to_csv('每户意愿.csv', index=False, encoding='gbk')
    print(f"\n共有 {len(valid_relocations)} 条符合条件的搬迁方案，已写入'每户意愿.csv'")

    return valid_relocations

def main():
    print("开始处理数据...")

    # 1. 加载数据
    distance_df, plots_df = load_data()

    # 2. 计算院落周边密度
    distance_df, ideal_density = calculate_courtyard_density(distance_df)

    # 3. 计算公共区域面积
    courtyard_data = calculate_public_space(plots_df)

    # 4. 准备搬迁评估所需的数据
    combined_data, ideal_density = prepare_relocation_data(plots_df, distance_df, courtyard_data, ideal_density)

    # 5. 分离有人居住的地块和空地块
    occupied_plots = combined_data[combined_data['是否有人居住'] == 1]
    empty_plots = combined_data[combined_data['是否有人居住'] == 0]
    print(f"\n共有 {len(occupied_plots)} 户居民和 {len(empty_plots)} 个空地块")

    # 6. 计算搬迁评分
    results_df = calculate_scores(occupied_plots, empty_plots, ideal_density)

    # 7. 计算repair并筛选符合条件的搬迁方案
    valid_relocations = calculate_repair_and_filter(results_df, plots_df)

    # 8. 简单统计分析
    if not valid_relocations.empty:
        print("\n最终方案评分统计：")
        print(valid_relocations[['面积评分', '采光评分', '街道距离评分', '密度评分', '公共空间评分', '总评分', 'repair']].describe())

        # 为每户居民找出最佳搬迁方案（repair最小的）
        # 按居民地块编号分组，对每组找出repair最小的记录
        best_options = valid_relocations.loc[valid_relocations.groupby('居民地块编号')['repair'].idxmin()]
        print(f"\n每户居民的最佳搬迁方案数量: {len(best_options)}")
        
        # 修改部分：调整列的顺序，把目标评分放在总评分之后，并且不输出是否搬迁列
        columns_order = [col for col in best_options.columns if col != '是否搬迁']
        # 确保目标评分在总评分后面
        total_score_index = columns_order.index('总评分')
        target_score_index = columns_order.index('目标评分')
        columns_order.remove('目标评分')
        columns_order.insert(total_score_index + 1, '目标评分')
        
        # 使用调整后的列顺序保存
        best_options[columns_order].to_csv('最佳搬迁方案.csv', index=False, encoding='gbk')
        print("最佳搬迁方案数据已写入'最佳搬迁方案.csv'")

        # 统计最佳方案中选择不搬迁的居民数量
        not_relocate_count = sum(best_options['是否搬迁'] == 0)
        print(f"选择不搬迁的居民数量：{not_relocate_count}，占比：{not_relocate_count/len(best_options):.2%}")

        # 可视化部分 - repair分布
        plt.figure(figsize=(10, 6))
        plt.hist(valid_relocations[valid_relocations['是否搬迁'] == 1]['repair'], bins=30, alpha=0.7, color='lightgreen')
        plt.title('Repair分布 (仅搬迁方案)')
        plt.xlabel('Repair值')
        plt.ylabel('频数')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.savefig('Repair分布.png', dpi=300, bbox_inches='tight')
        print("Repair分布图已保存为'Repair分布.png'")
    else:
        print("未找到符合条件的搬迁方案")

    print("\n数据处理完成!")

if __name__ == "__main__":
    main()
