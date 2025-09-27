import pandas as pd
import csv
import time
import random
from collections import defaultdict

# 步骤1：加载数据
def load_data():
    try:
        # 尝试不同编码加载数据
        try:
            data_df = pd.read_csv('每户意愿.csv', encoding='gbk')
            adjoin_df = pd.read_csv('adjoin.csv', encoding='gbk')
            plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gbk')
            print("成功使用GBK编码读取文件")
        except:
            try:
                data_df = pd.read_csv('每户意愿.csv', encoding='gb2312')
                adjoin_df = pd.read_csv('adjoin.csv', encoding='gb2312')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gb2312')
                print("成功使用GB2312编码读取文件")
            except:
                data_df = pd.read_csv('每户意愿.csv', encoding='ansi')
                adjoin_df = pd.read_csv('adjoin.csv', encoding='ansi')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='ansi')
                print("成功使用ANSI编码读取文件")
                
        # 重命名列以匹配之前的代码
        data_df = data_df.rename(columns={
            '居民地块编号': 'Id',
            '目标地块编号': 'newId',
            '总评分': 'Utotal',
            'repair': 'repair'
        })
        
        # 只保留需要的列
        data_df = data_df[['Id', 'newId', 'Utotal', 'repair']]
        
        return data_df, adjoin_df, plots_df
    except Exception as e:
        print(f"加载数据时出错: {e}")
        raise

# 创建查找表以提高性能
def create_lookups(plots_df, adjoin_df):
    # 创建按地块ID索引地块的查找表
    plots_by_id = {}
    for i, row in plots_df.iterrows():
        plot_id = row['地块ID']
        if plot_id not in plots_by_id:
            plots_by_id[plot_id] = []
        plots_by_id[plot_id].append(i)
    
    # 创建按院落ID索引地块的查找表
    courtyard_to_plots = {}
    for i, row in plots_df.iterrows():
        courtyard_id = row['院落ID']
        if courtyard_id not in courtyard_to_plots:
            courtyard_to_plots[courtyard_id] = []
        courtyard_to_plots[courtyard_id].append(i)
    
    # 创建相邻关系的查找表
    adjacency_lookup = {}
    for _, row in adjoin_df.iterrows():
        courtyard_id = row['Id']
        adjacencies = []
        for col in ['AdjoinId1', 'AdjoinId2', 'AdjoinId3', 'AdjoinId4', 'AdjoinId5']:
            if pd.notna(row[col]):
                adjacencies.append(int(row[col]))
        adjacency_lookup[courtyard_id] = adjacencies
    
    # 创建院落面积的查找表
    area_lookup = plots_df.drop_duplicates('院落ID').set_index('院落ID')['院落面积'].to_dict()
    
    return plots_by_id, courtyard_to_plots, adjacency_lookup, area_lookup

# 评估一个解决方案的适应度
def evaluate_solution(solution, plots_df, plots_by_id, courtyard_to_plots, adjacency_lookup, area_lookup):
    # 根据映射更新占用状态
    updated_occupancy = plots_df['是否有住户'].copy()
    
    # 先清空所有搬迁居民的原地块
    for id_val, new_id in solution:
        if id_val in plots_by_id:
            id_indices = plots_by_id[id_val]
            if any(updated_occupancy.iloc[idx] == 1 for idx in id_indices):
                for idx in id_indices:
                    updated_occupancy.iloc[idx] = 0
    
    # 然后分配新地块
    for id_val, new_id in solution:
        if new_id in plots_by_id:
            new_id_indices = plots_by_id[new_id]
            if all(updated_occupancy.iloc[idx] == 0 for idx in new_id_indices):
                updated_occupancy.iloc[new_id_indices[0]] = 1
    
    # 计算空院落
    empty_courtyards = []
    for courtyard_id, plot_indices in courtyard_to_plots.items():
        if all(updated_occupancy.iloc[idx] == 0 for idx in plot_indices):
            empty_courtyards.append(courtyard_id)
    
    # 计算Target1（空院落数量）
    target1 = len(empty_courtyards)
    
    # 计算Target2
    if not empty_courtyards:
        target2 = 0
    else:
        # 计算每个空院落相邻的空院落数量
        adjacent_empty_counts = []
        for courtyard in empty_courtyards:
            adjacent_courtyards = adjacency_lookup.get(courtyard, [])
            adj_empty_count = sum(1 for adj in adjacent_courtyards if adj in empty_courtyards)
            adjacent_empty_counts.append(adj_empty_count)
        
        max_adjacent = max(adjacent_empty_counts) if adjacent_empty_counts else 0
        sum_adjacent = sum(adjacent_empty_counts)
        
        # 计算空院落的面积
        empty_courtyard_areas = [area_lookup.get(c, 0) for c in empty_courtyards]
        sum_area = sum(empty_courtyard_areas)
        max_area = max(empty_courtyard_areas) if empty_courtyard_areas else 1
        
        # 计算Target2
        target2_part1 = sum_adjacent / max_adjacent if max_adjacent > 0 else 0
        target2_part2 = sum_area / max_area if max_area > 0 else 0
        target2 = target2_part1 + target2_part2
    
    # 计算Target3（Id != newId的映射数量）
    target3 = sum(1 for id_val, new_id in solution if id_val != new_id)
    
    return target1, target2, target3

# 创建真正随机的初始解
def create_random_solution(data_df, randomness=1.0):
    # 按Id分组获取每个居民的可选地块
    residents = defaultdict(list)
    for _, row in data_df.iterrows():
        residents[row['Id']].append((row['newId'], row['Utotal']))
    
    # 已分配的新地块集合
    assigned_plots = set()
    solution = []
    
    # 为每个居民分配一个未被分配的新地块
    for id_val, options_with_scores in residents.items():
        # 对选项进行随机排序，确保完全随机
        options = [opt[0] for opt in options_with_scores]
        
        # 根据randomness参数决定是否对所有选项完全随机化
        # randomness=1.0 表示完全随机
        # randomness=0.0 表示按原顺序（默认不搬迁会被优先选择）
        if random.random() < randomness:
            random.shuffle(options)
        
        # 过滤出未被分配的选项
        available_options = [opt for opt in options if opt not in assigned_plots]
        
        if not available_options:
            # 如果没有可用选项，尝试使用原地块（即使违反约束）
            if id_val not in assigned_plots:
                new_id = id_val
            else:
                # 如果原地块也被分配了，随机选择一个选项（违反约束，后续会修复）
                new_id = random.choice(options)
        else:
            # 随机选择一个未分配的选项
            new_id = random.choice(available_options)
        
        solution.append((id_val, new_id))
        assigned_plots.add(new_id)
    
    return solution

# 修复解决方案，确保每个新地块只分配给一个居民
def repair_solution(solution, data_df=None):
    assigned_plots = {}  # {新地块ID: (索引, 原地块)}
    
    # 检测冲突并记录
    for i, (id_val, new_id) in enumerate(solution):
        if new_id in assigned_plots:
            # 记录冲突，包含索引和优先级信息
            # 优先级：原地不动的居民优先保留
            prev_idx, prev_id = assigned_plots[new_id]
            prev_priority = 1 if prev_id == new_id else 0
            curr_priority = 1 if id_val == new_id else 0
            
            # 如果当前居民优先级更高，移除之前的分配
            if curr_priority > prev_priority:
                # 需要为之前的居民重新分配
                assigned_plots[new_id] = (i, id_val)
                solution[prev_idx] = (prev_id, prev_id)  # 先设为原地不动
            else:
                # 当前居民需要重新分配
                solution[i] = (id_val, id_val)  # 先设为原地不动
        else:
            assigned_plots[new_id] = (i, id_val)
    
    # 检查最终结果中是否有冲突
    final_assigned = set()
    has_conflict = False
    
    for id_val, new_id in solution:
        if new_id in final_assigned:
            has_conflict = True
            break
        final_assigned.add(new_id)
    
    if has_conflict and data_df is not None:
        # 如果还有冲突且提供了数据，尝试更复杂的修复
        return advanced_repair(solution, data_df)
    
    return solution

# 高级修复函数，尝试找到更好的替代解决方案
def advanced_repair(solution, data_df):
    # 统计每个新地块的分配情况
    new_id_count = {}
    
    for _, new_id in solution:
        if new_id in new_id_count:
            new_id_count[new_id] += 1
        else:
            new_id_count[new_id] = 1
    
    # 找出所有存在冲突的新地块
    conflict_plots = {new_id for new_id, count in new_id_count.items() if count > 1}
    
    # 创建每个居民的候选地块映射
    resident_options = defaultdict(list)
    for _, row in data_df.iterrows():
        resident_options[row['Id']].append((row['newId'], row['Utotal']))
    
    # 创建已分配地块的集合
    assigned_plots = {new_id for _, new_id in solution}
    
    # 修复解决方案
    fixed_solution = []
    for i, (id_val, new_id) in enumerate(solution):
        if new_id in conflict_plots:
            # 这个地块有冲突，需要重新分配
            
            # 获取居民的所有可选地块和评分
            options = resident_options.get(id_val, [])
            
            # 对选项按评分排序（评分高的优先）
            options.sort(key=lambda x: x[1], reverse=True)
            
            found_alternative = False
            for opt_id, _ in options:
                # 如果这个选项没有被分配，使用它
                if opt_id not in assigned_plots:
                    new_assignment = (id_val, opt_id)
                    fixed_solution.append(new_assignment)
                    assigned_plots.add(opt_id)
                    found_alternative = True
                    break
            
            if not found_alternative:
                # 如果没有找到可用的替代选项，保持原地不动
                new_assignment = (id_val, id_val)
                fixed_solution.append(new_assignment)
                assigned_plots.add(id_val)
        else:
            # 这个地块没有冲突，保持不变
            fixed_solution.append((id_val, new_id))
    
    return fixed_solution

# 遗传算法交叉操作
def crossover(parent1, parent2, data_df):
    # 创建子代，初始为父代1的复制
    child = parent1.copy()
    
    # 有50%的概率交换每个位置的基因
    for i in range(len(parent1)):
        if random.random() < 0.5:
            child[i] = parent2[i]
    
    # 修复任何违反约束的情况
    return repair_solution(child, data_df)

# 遗传算法变异操作
def mutate(solution, data_df, mutation_rate=0.1):
    # 创建一个解决方案副本
    mutated = solution.copy()
    
    # 为每个居民的决策进行变异
    for i, (id_val, _) in enumerate(solution):
        if random.random() < mutation_rate:
            # 获取该居民的所有可选地块
            options = data_df[data_df['Id'] == id_val]['newId'].tolist()
            if options:
                # 随机选择一个新的地块
                mutated[i] = (id_val, random.choice(options))
    
    # 修复任何违反约束的情况
    return repair_solution(mutated, data_df)

# 遗传算法主函数
def genetic_algorithm(data_df, adjoin_df, plots_df, population_size=50, generations=100):
    print("正在运行遗传算法...")
    
    # 创建查找表
    plots_by_id, courtyard_to_plots, adjacency_lookup, area_lookup = create_lookups(plots_df, adjoin_df)
    
    # 创建初始种群 - 使用真正随机的生成方法
    print("正在创建初始种群...")
    population = []
    for _ in range(population_size):
        # 使用randomness=0.8，表示80%的情况下完全随机选择
        # 这样既能提供多样性，又能保留一些原本可能是好的不搬迁选项
        solution = create_random_solution(data_df, randomness=0.8)
        population.append(solution)
    
    best_solution = None
    best_target1 = -1
    best_target2 = -1
    best_target3 = float('inf')
    
    start_time = time.time()
    
    # 迭代进化
    for gen in range(generations):
        # 评估种群中每个解决方案的适应度
        fitness_values = []
        for solution in population:
            target1, target2, target3 = evaluate_solution(
                solution, plots_df, plots_by_id, courtyard_to_plots, adjacency_lookup, area_lookup
            )
            fitness_values.append((target1, target2, target3))
            
            # 更新最佳解
            if (target1 > best_target1 or 
                (target1 == best_target1 and target2 > best_target2) or 
                (target1 == best_target1 and target2 == best_target2 and target3 < best_target3)):
                best_solution = solution
                best_target1 = target1
                best_target2 = target2
                best_target3 = target3
        
        # 进度报告
        if gen % 10 == 0 or gen == generations - 1:
            print(f"代数 {gen+1}/{generations} - " 
                  f"当前最佳: T1={best_target1}, T2={best_target2:.4f}, T3={best_target3} - "
                  f"耗时: {time.time() - start_time:.2f}秒")
            # 打印当前最佳解中移动的居民比例
            if best_solution:
                move_count = sum(1 for id_val, new_id in best_solution if id_val != new_id)
                move_percent = move_count / len(best_solution) * 100
                print(f"  移动居民: {move_count}/{len(best_solution)} ({move_percent:.2f}%)")
        
        # 如果达到最后一代，跳出循环
        if gen == generations - 1:
            break
        
        # 选择下一代
        new_population = []
        
        # 精英保留策略 - 保留最好的解决方案
        elite_count = max(1, int(population_size * 0.1))
        elite_indices = sorted(range(len(fitness_values)), 
                              key=lambda i: (fitness_values[i][0], fitness_values[i][1], -fitness_values[i][2]), 
                              reverse=True)[:elite_count]
        
        for idx in elite_indices:
            new_population.append(population[idx])
        
        # 生成其余解决方案
        while len(new_population) < population_size:
            # 选择父代（锦标赛选择）
            tournament_size = 3
            p1_idx = max(random.sample(range(len(population)), tournament_size), 
                        key=lambda i: (fitness_values[i][0], fitness_values[i][1], -fitness_values[i][2]))
            p2_idx = max(random.sample(range(len(population)), tournament_size), 
                        key=lambda i: (fitness_values[i][0], fitness_values[i][1], -fitness_values[i][2]))
            
            parent1 = population[p1_idx]
            parent2 = population[p2_idx]
            
            # 交叉和变异
            child = crossover(parent1, parent2, data_df)
            child = mutate(child, data_df)
            
            new_population.append(child)
        
        population = new_population
    
    print(f"遗传算法完成，总耗时: {time.time() - start_time:.2f}秒")
    return best_solution, best_target1, best_target2, best_target3

# 将输出写入CSV文件
def write_output(best_combination):
    with open('output.csv', 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Id', 'newId'])
        for id_val, new_id in best_combination:
            writer.writerow([id_val, new_id])

# ============= 新增代码：计算搬迁成本和收益 =============

# 计算空置院落和搬迁成本
def calculate_empty_courtyards_and_costs(solution, data_df, plots_df, courtyard_to_plots):
    """
    计算完全空置的院落、搬迁成本和收益
    
    参数:
    solution - 当前解决方案，格式为 [(Id, newId), ...]
    data_df - 居民意愿数据
    plots_df - 地块信息数据
    courtyard_to_plots - 院落到地块的映射
    
    返回:
    empty_courtyards - 完全空置的院落ID列表
    total_cost - 总搬迁成本
    updated_occupancy - 更新后的占用状态
    initial_occupancy - 初始占用状态
    """
    # 创建一个字典，记录每个地块ID对应的repair值
    repair_costs = {}
    for _, row in data_df.iterrows():
        id_val = row['Id']
        new_id = row['newId']
        repair = row['repair']
        repair_costs[(id_val, new_id)] = repair
    
    # 获取每个地块的初始状态
    initial_occupancy = {}
    for _, row in plots_df.iterrows():
        plot_id = row['地块ID']
        occupied = row['是否有住户']
        initial_occupancy[plot_id] = occupied
    
    # 更新后的占用状态
    updated_occupancy = initial_occupancy.copy()
    
    # 记录被搬迁的居民地块
    moved_residents = []
    
    # 根据方案更新占用状态
    for id_val, new_id in solution:
        if id_val != new_id:  # 只考虑搬迁的居民
            # 清空原地块
            if id_val in updated_occupancy and updated_occupancy[id_val] == 1:
                updated_occupancy[id_val] = 0
                moved_residents.append(id_val)
            
            # 设置新地块为占用
            if new_id in updated_occupancy:
                updated_occupancy[new_id] = 1
    
    # 计算完全空置的院落
    empty_courtyards = []
    for courtyard_id, plot_indices in courtyard_to_plots.items():
        plots_in_courtyard = []
        for idx in plot_indices:
            plot_id = plots_df.iloc[idx]['地块ID']
            plots_in_courtyard.append(plot_id)
        
        # 检查这个院落的所有地块是否都空置
        if all(updated_occupancy.get(plot_id, 0) == 0 for plot_id in plots_in_courtyard):
            empty_courtyards.append(courtyard_id)
    
    # 计算总搬迁成本
    total_cost = 0
    # 加上修缮费用
    for id_val, new_id in solution:
        if id_val != new_id:  # 只考虑搬迁的居民
            # 修缮费用
            repair_cost = repair_costs.get((id_val, new_id), 0)
            # 沟通费用（每户30000）
            communication_cost = 30000
            total_cost += repair_cost + communication_cost
    
    return empty_courtyards, total_cost, updated_occupancy, initial_occupancy

# 计算收入
def calculate_income(empty_courtyards, plots_df, updated_occupancy, initial_occupancy, 
                     courtyard_to_plots, adjacency_lookup):
    """
    根据公式计算收入
    
    money = ∑(3650 - 120x_j)(1 - x_j')(Y_{k_j}N_jA_j + (I(S_{k_j} > 0) * 0.2 + 1)(1 - Y_{k_j})30A_j')
    
    参数:
    empty_courtyards - 完全空置的院落ID列表
    plots_df - 地块信息数据
    updated_occupancy - 更新后的地块占用状态
    initial_occupancy - 初始地块占用状态
    courtyard_to_plots - 院落到地块的映射
    adjacency_lookup - 院落相邻关系
    
    返回:
    income - 总收入
    """
    total_income = 0
    
    # 创建院落空置状态映射
    courtyard_empty_status = {}
    for courtyard_id in set(plots_df['院落ID']):
        courtyard_empty_status[courtyard_id] = 1 if courtyard_id in empty_courtyards else 0
    
    # 创建一个映射，记录每个地块所属的院落
    plot_to_courtyard = {}
    courtyard_areas = {}  # 记录院落面积
    courtyard_plots = defaultdict(list)  # 记录每个院落包含的地块
    
    for _, row in plots_df.iterrows():
        plot_id = row['地块ID']
        courtyard_id = row['院落ID']
        plot_to_courtyard[plot_id] = courtyard_id
        courtyard_areas[courtyard_id] = row['院落面积']
        courtyard_plots[courtyard_id].append(plot_id)
    
    # 计算每个院落的所有地块面积总和
    courtyard_plot_areas = defaultdict(float)
    for _, row in plots_df.iterrows():
        plot_id = row['地块ID']
        courtyard_id = row['院落ID']
        area = row['地块面积']
        courtyard_plot_areas[courtyard_id] += area
    
    # 计算A_j'的调整值: (院落面积 - 所有地块面积总和) / 地块数量
    courtyard_area_adjustments = {}
    for courtyard_id, total_plot_area in courtyard_plot_areas.items():
        courtyard_area = courtyard_areas.get(courtyard_id, 0)
        num_plots = len(courtyard_plots[courtyard_id])
        adjustment = (courtyard_area - total_plot_area) / num_plots if num_plots > 0 else 0
        # 如果调整值为负，则设为0
        adjustment = max(0, adjustment)
        courtyard_area_adjustments[courtyard_id] = adjustment
    
    # 计算每个院落相邻的空院落数量
    courtyard_adjacent_empty_count = {}
    for courtyard_id in set(plots_df['院落ID']):
        adjacent_courtyards = adjacency_lookup.get(courtyard_id, [])
        count = sum(1 for adj in adjacent_courtyards if adj in empty_courtyards)
        courtyard_adjacent_empty_count[courtyard_id] = count
    
    # 计算每个地块的收入贡献
    for _, row in plots_df.iterrows():
        plot_id = row['地块ID']
        courtyard_id = row['院落ID']
        area = row['地块面积']
        orientation = row['地块方位']
        
        # 获取x_j（初始占用状态）
        x_j = initial_occupancy.get(plot_id, 0)
        
        # 获取x_j'（最终占用状态）
        x_j_prime = updated_occupancy.get(plot_id, 0)
        
        # 获取Y_{k_j}（院落空置状态）
        Y_k_j = courtyard_empty_status.get(courtyard_id, 0)
        
        # 获取N_j（方位系数）
        if orientation in ['东', '西']:
            N_j = 8
        elif orientation in ['南', '北']:
            N_j = 15
        else:
            N_j = 0  # 默认值，应该不会出现
        
        # 获取A_j（地块面积）
        A_j = area
        
        # 计算A_j'（调整后的地块面积）
        adjustment = courtyard_area_adjustments.get(courtyard_id, 0)
        A_j_prime = A_j + adjustment
        
        # 获取S_{k_j}（相邻空院落数量）
        S_k_j = courtyard_adjacent_empty_count.get(courtyard_id, 0)
        
        # 计算I(S_{k_j} > 0)
        I_S_k_j = 1 if S_k_j > 0 else 0
        
        # 计算公式部分
        part1 = (3650 - 120 * x_j) * (1 - x_j_prime)
        part2 = Y_k_j * N_j * A_j
        part3 = (I_S_k_j * 0.2 + 1) * (1 - Y_k_j) * 30 * A_j_prime
        
        # 计算这个地块的收入贡献
        income_contribution = part1 * (part2 + part3)
        total_income += income_contribution
    
    return total_income

# 计算空置院落的总面积
def calculate_empty_courtyards_area(empty_courtyards, area_lookup):
    """
    计算所有空置院落的总面积
    
    参数:
    empty_courtyards - 空置院落ID列表
    area_lookup - 院落ID到面积的映射
    
    返回:
    total_area - 所有空置院落的总面积
    """
    total_area = 0
    for courtyard_id in empty_courtyards:
        area = area_lookup.get(courtyard_id, 0)
        total_area += area
    return total_area

# 主函数，包含计算空置院落、成本和收益的功能
def main_with_analysis():
    try:
        # 加载数据
        data_df, adjoin_df, plots_df = load_data()
        
        # 打印数据统计
        id_counts = data_df['Id'].value_counts()
        print(f"不同Id的数量: {len(id_counts)}")
        print(f"映射选项总数: {len(data_df)}")
        
        # 分析数据集中不搬迁的比例
        stay_count = sum(1 for _, row in data_df.iterrows() if row['Id'] == row['newId'])
        print(f"原地不动的选项: {stay_count}/{len(data_df)} ({stay_count/len(data_df)*100:.2f}%)")
        
        # 创建查找表
        plots_by_id, courtyard_to_plots, adjacency_lookup, area_lookup = create_lookups(plots_df, adjoin_df)
        
        # 运行遗传算法找最优解
        best_combination, target1, target2, target3 = genetic_algorithm(
            data_df, adjoin_df, plots_df, population_size=100, generations=200
        )
        
        # 分析最优解
        print(f"\n找到最优映射:")
        print(f"Target1（空院落数量）: {target1}")
        print(f"Target2（相邻性+面积比率）: {target2:.4f}")
        print(f"Target3（映射数量）: {target3}")
        
        # 计算最优解中搬迁的居民比例
        if best_combination:
            move_count = sum(1 for id_val, new_id in best_combination if id_val != new_id)
            total = len(best_combination)
            print(f"搬迁居民比例: {move_count}/{total} ({move_count/total*100:.2f}%)")
            
            # 计算空置院落、成本和收益
            empty_courtyards, total_cost, updated_occupancy, initial_occupancy = calculate_empty_courtyards_and_costs(
                best_combination, data_df, plots_df, courtyard_to_plots
            )
            
            # 计算空置院落总面积
            empty_courtyards_area = calculate_empty_courtyards_area(empty_courtyards, area_lookup)
            
            # 计算总收入
            total_income = calculate_income(
                empty_courtyards, plots_df, updated_occupancy, initial_occupancy, 
                courtyard_to_plots, adjacency_lookup
            )
            
            # 计算净利润
            profit = total_income - total_cost
            
            # 输出结果
            print("\n========== 搬迁方案分析 ==========")
            print(f"完全空置的院落数量: {len(empty_courtyards)}")
            print(f"完全空置的院落ID: {sorted(empty_courtyards)}")
            print(f"完全空置院落的总面积: {empty_courtyards_area:.2f}平方米")
            print(f"总投入成本: {total_cost:.2f}元")
            print(f"  - 其中修缮费: {total_cost - move_count * 30000:.2f}元")
            print(f"  - 其中沟通费: {move_count * 30000:.2f}元")
            print(f"总收入: {total_income:.2f}元")
            print(f"净利润: {profit:.2f}元")
            
        # 写入输出
        write_output(best_combination)
        print("结果已写入output.csv")
        
    except Exception as e:
        print(f"程序运行出错: {e}")
        import traceback
        traceback.print_exc()

# 如果是主程序，则执行main_with_analysis函数
if __name__ == "__main__":
    main_with_analysis()
