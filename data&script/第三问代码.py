import pandas as pd
import csv
import random
import time
from collections import defaultdict

# 修改后的load_data函数
def load_data():
    """
    加载必要的数据文件，尝试多种编码方式
    """
    try:
        # 尝试用不同编码加载数据
        try:
            # 加载每户意愿.csv - 映射可能性
            data_df = pd.read_csv('问题一最终搬迁方案表-提供修缮补偿.csv', encoding='gbk')
            # 加载adjoin.csv - 院落相邻关系信息
            adjoin_df = pd.read_csv('adjoin.csv', encoding='gbk')
            # 加载附件一：老城街区地块信息.csv - 地块和院落信息
            plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gbk')
            print("成功使用GBK编码读取文件")
        except:
            try:
                # 如果GBK失败，尝试使用GB2312
                data_df = pd.read_csv('问题一最终搬迁方案表-提供修缮补偿.csv', encoding='gb2312')
                adjoin_df = pd.read_csv('adjoin.csv', encoding='gb2312')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='gb2312')
                print("成功使用GB2312编码读取文件")
            except:
                # 如果GB2312也失败，尝试指定ANSI
                data_df = pd.read_csv('问题一最终搬迁方案表-提供修缮补偿.csv', encoding='ansi')
                adjoin_df = pd.read_csv('adjoin.csv', encoding='ansi')
                plots_df = pd.read_csv('附件一：老城街区地块信息.csv', encoding='ansi')
                print("成功使用ANSI编码读取文件")
        
        # 重命名列以匹配之前的代码
        data_df = data_df.rename(columns={
            'PlotID': 'Id',
            'newPlotID': 'newId',
            'TotalWillingness': 'Utotal',
            'repair': 'repair'
        })
        
        # 检查并确保数据中有repair列
        if 'repair' not in data_df.columns and len(data_df.columns) >= 4:
            data_df = data_df.rename(columns={data_df.columns[3]: 'repair'})
        
        return data_df, adjoin_df, plots_df
    
    except Exception as e:
        print(f"加载数据时出错: {e}")
        raise

# 数据预处理
def preprocess_data(plots_df, adjoin_df):
    """
    预处理地块和院落数据，创建必要的计算字段
    """
    # 处理地块信息
    block_df = plots_df.copy()
    
    # 重命名列以便于后续使用
    column_names = {
        block_df.columns[0]: 'block_id',  # 第一列是地块id
        block_df.columns[1]: 'yard_id',   # 第二列是院落id
        block_df.columns[2]: 'area',      # 第三列是地块面积
        block_df.columns[3]: 'yard_area', # 第四列是院落面积
        block_df.columns[4]: 'direction', # 第五列是方位
        block_df.columns[5]: 'occupied'   # 第六列是是否有住户
    }
    block_df = block_df.rename(columns=column_names)
    
    # 将方位转换为数值 N_j
    direction_map = {'East': 8, 'West': 8, 'South': 15, 'North': 15}
    block_df['N_j'] = block_df['direction'].map(lambda x: direction_map.get(x, 0))
    
    # 计算每个院落的总地块面积
    yard_block_area = block_df.groupby('yard_id')['area'].sum()
    
    # 计算每个院落包含的地块数
    yard_block_count = block_df.groupby('yard_id').size()
    
    # 计算 A_j'
    block_df['A_j_prime'] = block_df.apply(
        lambda row: max(0, (row['yard_area'] - yard_block_area[row['yard_id']]) / yard_block_count[row['yard_id']]) + row['area']
        if row['yard_id'] in yard_block_area.index else row['area'],
        axis=1
    )
    
    # 处理院落相邻关系
    # 适配adjoin_df的列名
    if len(adjoin_df.columns) >= 2:
        adjoin_columns = {
            adjoin_df.columns[0]: 'yard_id'
        }
        for i in range(1, min(6, len(adjoin_df.columns))):
            adjoin_columns[adjoin_df.columns[i]] = f'adjoin{i}'
        
        adjoin_df = adjoin_df.rename(columns=adjoin_columns)
    else:
        print(f"警告: adjoin.csv 列数不正确，请检查文件格式。当前列数: {len(adjoin_df.columns)}")
    
    # 创建院落相邻关系字典
    yard_adjoins = {}
    for _, row in adjoin_df.iterrows():
        adjoins = []
        for i in range(1, min(6, len(adjoin_df.columns))):
            col_name = f'adjoin{i}'
            if col_name in adjoin_df.columns and pd.notna(row[col_name]):
                adjoins.append(int(row[col_name]))
        yard_adjoins[int(row['yard_id'])] = adjoins
    
    # 创建查找表，方便后续计算
    # 地块ID到索引的映射
    block_id_to_index = {}
    for idx, row in block_df.iterrows():
        block_id = row['block_id']
        if block_id not in block_id_to_index:
            block_id_to_index[block_id] = []
        block_id_to_index[block_id].append(idx)
    
    # 院落ID到地块的映射
    yard_to_blocks = {}
    for _, row in block_df.iterrows():
        yard_id = row['yard_id']
        block_id = row['block_id']
        if yard_id not in yard_to_blocks:
            yard_to_blocks[yard_id] = []
        yard_to_blocks[yard_id].append(block_id)
    
    return block_df, yard_adjoins, block_id_to_index, yard_to_blocks

# 修改后的create_random_solution函数
def create_random_solution(data_df, occupied_plots, randomness=1.0):
    """
    创建一个随机的解决方案，确保所有有住户的地块都被考虑
    并且不会有冲突（即不允许两个不同的居民搬到同一个地块）
    
    参数:
    data_df - 包含每个居民可能搬迁选项的数据
    occupied_plots - 实际有住户的地块ID列表（113个）
    randomness - 随机性程度，1.0表示完全随机选择
    
    返回:
    randomly_chosen - 随机选择的解决方案
    """
    # 按Id分组获取每个居民的可选地块
    residents = defaultdict(list)
    for _, row in data_df.iterrows():
        residents[row['Id']].append((row['newId'], row.get('Utotal', 0)))
    
    # 确保所有有住户的地块都被考虑
    for plot_id in occupied_plots:
        if plot_id not in residents:
            # 如果有住户的地块在意愿数据中不存在，添加默认选项（原地不动）
            residents[plot_id].append((plot_id, 1.0))
    
    # 已分配的新地块集合
    assigned_plots = set()
    solution = []
    
    # 为每个有住户的地块随机分配一个目标地块
    resident_keys = list(occupied_plots)  # 只考虑有住户的地块
    # 随机打乱居民顺序，避免优先分配问题
    random.shuffle(resident_keys)
    
    for id_val in resident_keys:
        # 获取该地块的所有可能目标地块
        if id_val in residents:
            options_with_scores = residents[id_val]
            
            # 对选项进行排序
            if random.random() < randomness:
                # 随机排序
                random.shuffle(options_with_scores)
            else:
                # 按评分排序
                options_with_scores.sort(key=lambda x: x[1], reverse=True)
            
            # 过滤出未被分配的选项
            available_options = [opt[0] for opt in options_with_scores if opt[0] not in assigned_plots]
            
            if not available_options:
                # 如果没有可用选项，尝试使用原地块（如果未被分配）
                if id_val not in assigned_plots:
                    new_id = id_val
                else:
                    # 寻找任何未被分配的地块（优先选择无人地块）
                    unassigned_plots = set(range(1, 485)) - assigned_plots
                    if unassigned_plots:
                        new_id = random.choice(list(unassigned_plots))
                    else:
                        # 理论上不应该到这里，因为总共有484个地块
                        print(f"警告: 无法为地块{id_val}找到未分配的目标地块")
                        continue
            else:
                # 随机选择一个未分配的选项
                new_id = random.choice(available_options)
            
            solution.append((id_val, new_id))
            assigned_plots.add(new_id)
        else:
            # 这种情况不应该发生，因为我们已经确保所有有住户的地块都有选项
            print(f"警告: 地块{id_val}没有可选的目标地块")
    
    # 检查是否所有113个有住户的地块都被处理
    if len(solution) != len(occupied_plots):
        print(f"警告: 创建的解决方案长度不等于{len(occupied_plots)}，实际长度为{len(solution)}")
    
    return solution

# 修改后的repair_solution函数
def repair_solution(solution, occupied_plots):
    """
    修复解决方案中的冲突，确保:
    1. 每个新地块只分配给一个居民
    2. 所有有住户的地块都在解决方案中
    """
    # 已分配的新地块和对应的原地块
    assigned_plots = {}  # {新地块ID: (索引, 原地块ID)}
    
    # 第一遍：优先保留原地不动的分配
    for i, (id_val, new_id) in enumerate(solution):
        if new_id in assigned_plots:
            # 处理冲突
            prev_idx, prev_id = assigned_plots[new_id]
            
            # 优先级：原地不动 > 其他
            prev_priority = 1 if prev_id == new_id else 0
            curr_priority = 1 if id_val == new_id else 0
            
            if curr_priority > prev_priority:
                # 当前分配优先级更高，替换之前的
                assigned_plots[new_id] = (i, id_val)
                # 将之前的标记为需要重新分配
                solution[prev_idx] = (prev_id, None)
            else:
                # 保留之前的分配，当前的需要重新分配
                solution[i] = (id_val, None)
        else:
            # 无冲突，直接添加
            assigned_plots[new_id] = (i, id_val)
    
    # 第二遍：重新分配所有标记为需要重新分配的地块
    for i, (id_val, new_id) in enumerate(solution):
        if new_id is None:
            # 尝试原地不动（如果未被分配）
            if id_val not in assigned_plots:
                solution[i] = (id_val, id_val)
                assigned_plots[id_val] = (i, id_val)
            else:
                # 寻找任何未被分配的地块
                unassigned = set(range(1, 485)) - set(assigned_plots.keys())
                if unassigned:
                    # 选择第一个未分配的地块
                    new_unassigned = min(unassigned)
                    solution[i] = (id_val, new_unassigned)
                    assigned_plots[new_unassigned] = (i, id_val)
                else:
                    # 理论上不应该到这里
                    print(f"严重错误: 无法为地块{id_val}找到未分配的目标地块")
    
    # 确保所有有住户的地块都在解决方案中
    included_ids = set(id_val for id_val, _ in solution)
    missing_ids = set(occupied_plots) - included_ids
    
    if missing_ids:
        #print(f"警告: 有{len(missing_ids)}个有住户的地块不在解决方案中")
        
        # 为缺失的地块添加分配
        for id_val in missing_ids:
            # 尝试原地不动
            if id_val not in assigned_plots:
                solution.append((id_val, id_val))
                assigned_plots[id_val] = (len(solution)-1, id_val)
            else:
                # 寻找任何未被分配的地块
                unassigned = set(range(1, 485)) - set(assigned_plots.keys())
                if unassigned:
                    new_unassigned = min(unassigned)
                    solution.append((id_val, new_unassigned))
                    assigned_plots[new_unassigned] = (len(solution)-1, id_val)
                else:
                    print(f"严重错误: 无法为缺失地块{id_val}找到未分配的目标地块")
    
    # 最终检查：确保没有冲突和缺失
    final_target_plots = set()
    final_source_plots = set()
    has_conflicts = False
    
    for id_val, new_id in solution:
        final_source_plots.add(id_val)
        if new_id in final_target_plots:
            has_conflicts = True
            print(f"严重错误: 目标地块{new_id}被多次分配")
        final_target_plots.add(new_id)
    
    missing_sources = set(occupied_plots) - final_source_plots
    if missing_sources:
        print(f"严重错误: 有{len(missing_sources)}个有住户的地块不在最终解决方案中")
    
    if has_conflicts or missing_sources:
        print("修复失败，解决方案仍有问题")
    
    return solution




# 修正crossover函数，确保长度一致
def crossover(parent1, parent2):
    """
    执行两个父解决方案之间的交叉操作，并确保结果长度一致
    """
    # 创建一个字典，用于查找第二个父代中相应地块ID的映射
    parent2_dict = {id_val: new_id for id_val, new_id in parent2}
    
    # 创建子代，只包含parent1中的元素，但可能交换目标地块
    child = []
    for id_val, new_id in parent1:
        if random.random() < 0.5 and id_val in parent2_dict:
            # 使用parent2中对应地块的映射
            child.append((id_val, parent2_dict[id_val]))
        else:
            # 保留parent1中的映射
            child.append((id_val, new_id))
    
    # 确保长度一致
    assert len(child) == len(parent1), f"Crossover结果长度不一致: {len(child)} vs {len(parent1)}"
    
    return child

# 修改变异操作函数，添加修复步骤
def mutate(solution, data_df, mutation_rate=0.1):
    """
    对解决方案进行变异
    """
    # 创建解决方案副本
    mutated = solution.copy()
    
    # 为每个居民决策进行变异
    for i, (id_val, _) in enumerate(solution):
        if random.random() < mutation_rate:
            # 获取该居民的所有可选地块
            options = data_df[data_df['Id'] == id_val]['newId'].tolist()
            if options:
                # 随机选择一个新地块
                mutated[i] = (id_val, random.choice(options))
    
    # 返回变异后的解决方案（不在此处修复，由调用者处理）
    return mutated

# 计算给定映射的收入
def calculate_income(solution, block_df, yard_adjoins, block_id_to_index, yard_to_blocks):
    """
    计算给定映射的收入
    
    公式: money = ∑(3650 - 120x_j)(1 - x_j')(Y_{k_j}N_jA_j + (I(S_{k_j} > 0) * 0.2 + 1)(1 - Y_{k_j})30A_j')
    """
    # 复制初始状态
    current_occupancy = block_df['occupied'].copy()
    
    # 创建映射字典
    id_to_newid = {m[0]: m[1] for m in solution}
    
    # 执行所有搬迁
    for old_id, new_id in id_to_newid.items():
        if old_id != new_id:  # 只考虑搬迁，不考虑原地不动
            # 找到原地块和新地块的索引
            if old_id in block_id_to_index and new_id in block_id_to_index:
                # 获取第一个索引（假设一个地块ID只对应一行数据）
                old_idx = block_id_to_index[old_id][0]
                new_idx = block_id_to_index[new_id][0]
                
                # 如果原地块被占用且新地块空置，执行搬迁
                if current_occupancy[old_idx] == 1 and current_occupancy[new_idx] == 0:
                    current_occupancy[old_idx] = 0
                    current_occupancy[new_idx] = 1
    
    # 计算每个院落的空置状态
    yard_empty = {}
    for yard_id, block_ids in yard_to_blocks.items():
        all_empty = True
        for block_id in block_ids:
            if block_id in block_id_to_index:
                idx = block_id_to_index[block_id][0]
                if current_occupancy[idx] == 1:
                    all_empty = False
                    break
        yard_empty[yard_id] = all_empty
    
    # 计算每个院落相邻的空院落数量
    yard_empty_neighbors = {}
    for yard_id in yard_empty:
        if yard_id in yard_adjoins:
            empty_count = sum(1 for adj in yard_adjoins[yard_id] if adj in yard_empty and yard_empty[adj])
            yard_empty_neighbors[yard_id] = empty_count
        else:
            yard_empty_neighbors[yard_id] = 0
    
    # 计算目标值
    total_income = 0
    for idx, block in block_df.iterrows():
        block_id = block['block_id']
        yard_id = block['yard_id']
        
        # 获取x_j（初始占用状态）
        x_j = block['occupied']
        
        # 获取x_j'（最终占用状态）
        x_j_prime = current_occupancy[idx]
        
        # 获取Y_{k_j}（院落空置状态）
        Y_k_j = 1 if yard_empty.get(yard_id, False) else 0
        
        # 获取N_j（方位系数）
        N_j = block['N_j']
        
        # 获取A_j（地块面积）
        A_j = block['area']
        
        # 获取A_j'（调整后的地块面积）
        A_j_prime = block['A_j_prime']
        
        # 获取S_{k_j}（相邻空院落数量）
        S_k_j = yard_empty_neighbors.get(yard_id, 0)
        
        # 计算公式部分
        part1 = (3650 - 120 * x_j) * (1 - x_j_prime)
        part2 = Y_k_j * N_j * A_j
        part3 = ((1 if S_k_j > 0 else 0) * 0.2 + 1) * (1 - Y_k_j) * 30 * A_j_prime
        
        # 计算这个地块的收入贡献
        income_contribution = part1 * (part2 + part3)
        total_income += income_contribution
    
    # 返回院落空置信息和总收入
    return total_income, yard_empty, current_occupancy

# 计算搬迁成本
def calculate_costs(solution, data_df):
    """
    计算总搬迁成本
    """
    # 创建一个字典，记录每个地块ID的repair值
    repair_costs = {}
    for _, row in data_df.iterrows():
        id_val = row['Id']
        new_id = row['newId']
        repair = row.get('repair', 0)  # 确保repair列存在
        repair_costs[(id_val, new_id)] = repair
    
    # 计算总搬迁成本
    total_cost = 0
    for id_val, new_id in solution:
        if id_val != new_id:  # 只考虑搬迁的居民
            # 修缮费
            repair_cost = repair_costs.get((id_val, new_id), 0)
            # 沟通费（每户30000）
            communication_cost = 30000
            total_cost += repair_cost + communication_cost
    
    return total_cost

# 计算空置院落总面积
def calculate_empty_yards_area(yard_empty, block_df):
    """
    计算所有空置院落的总面积
    """
    empty_yards = [yard_id for yard_id, is_empty in yard_empty.items() if is_empty]
    
    # 获取每个空置院落的面积
    yard_areas = block_df.drop_duplicates('yard_id').set_index('yard_id')['yard_area'].to_dict()
    
    # 计算总面积
    total_area = sum(yard_areas.get(yard_id, 0) for yard_id in empty_yards)
    
    return total_area, empty_yards

#计算不搬迁时的基准收益
def calculate_baseline_income(block_df, yard_adjoins, block_id_to_index, yard_to_blocks):
    """
    计算不实行搬迁方案时的基准收益
    """
    # 使用原始占用状态作为最终状态
    original_occupancy = block_df['occupied'].copy()
    
    # 计算每个院落的空置状态
    yard_empty = {}
    for yard_id, block_ids in yard_to_blocks.items():
        all_empty = True
        for block_id in block_ids:
            if block_id in block_id_to_index:
                idx = block_id_to_index[block_id][0]
                if original_occupancy[idx] == 1:
                    all_empty = False
                    break
        yard_empty[yard_id] = all_empty
    
    # 计算每个院落相邻的空院落数量
    yard_empty_neighbors = {}
    for yard_id in yard_empty:
        if yard_id in yard_adjoins:
            empty_count = sum(1 for adj in yard_adjoins[yard_id] if adj in yard_empty and yard_empty[adj])
            yard_empty_neighbors[yard_id] = empty_count
        else:
            yard_empty_neighbors[yard_id] = 0
    
    # 计算目标值
    total_income = 0
    for idx, block in block_df.iterrows():
        block_id = block['block_id']
        yard_id = block['yard_id']
        
        # 获取x_j（初始占用状态）
        x_j = block['occupied']
        
        # 获取x_j'（最终占用状态 - 在不搬迁情况下与初始状态相同）
        x_j_prime = original_occupancy[idx]
        
        # 获取Y_{k_j}（院落空置状态）
        Y_k_j = 1 if yard_empty.get(yard_id, False) else 0
        
        # 获取N_j（方位系数）
        N_j = block['N_j']
        
        # 获取A_j（地块面积）
        A_j = block['area']
        
        # 获取A_j'（调整后的地块面积）
        A_j_prime = block['A_j_prime']
        
        # 获取S_{k_j}（相邻空院落数量）
        S_k_j = yard_empty_neighbors.get(yard_id, 0)
        
        # 计算公式部分
        part1 = (3650 - 120 * x_j) * (1 - x_j_prime)
        part2 = Y_k_j * N_j * A_j
        part3 = ((1 if S_k_j > 0 else 0) * 0.2 + 1) * (1 - Y_k_j) * 30 * A_j_prime
        
        # 计算这个地块的收入贡献
        income_contribution = part1 * (part2 + part3)
        total_income += income_contribution
    
    return total_income


# 遗传算法主函数
def genetic_algorithm(data_df, block_df, yard_adjoins, block_id_to_index, yard_to_blocks, 
                     population_size=50, generations=100, elite_ratio=0.2):
    """
    使用遗传算法优化搬迁方案
    """
    print("正在运行遗传算法...")
    
    # 获取所有有住户的地块ID
    occupied_plots = set(block_df[block_df['occupied'] == 1]['block_id'].unique())
    occupied_plots_list = sorted(list(occupied_plots))  # 转换为排序列表以保持一致性
    print(f"发现{len(occupied_plots)}个有住户的地块")
    
    # 创建初始种群，确保每个解决方案只包含113个有住户地块的映射
    print("创建初始种群...")
    population = []
    for _ in range(population_size):
        solution = create_random_solution(data_df, occupied_plots, randomness=0.8)
        solution = repair_solution(solution, occupied_plots)
        
        # 额外检查，确保解决方案只包含113个映射，且只包含有住户的地块
        solution_dict = {id_val: new_id for id_val, new_id in solution}
        correct_solution = [(id_val, solution_dict.get(id_val, id_val)) for id_val in occupied_plots_list]
        
        # 确保长度一致
        assert len(correct_solution) == len(occupied_plots), f"解决方案长度不等于{len(occupied_plots)}"
        
        population.append(correct_solution)
    #创建初始种群后的检查
    for i in range(len(population)):
        population[i] = ensure_correct_solution(population[i], occupied_plots)
    # 计算基准收益（不搬迁的情况）
    baseline_income = calculate_baseline_income(block_df, yard_adjoins, block_id_to_index, yard_to_blocks)
    print(f"不实行搬迁方案的基准收益: {baseline_income:.2f}元")
    
    best_solution = None
    best_income = float('-inf')
    best_cost = float('inf')
    best_profit = float('-inf')
    best_cost_effectiveness = float('-inf')
    
    start_time = time.time()
    
    # 迭代进化
    for gen in range(generations):
        # 评估种群中每个解决方案的适应度
        fitness_values = []
        
        for solution in population:
            income, _, _ = calculate_income(solution, block_df, yard_adjoins, block_id_to_index, yard_to_blocks)
            cost = calculate_costs(solution, data_df)
            profit = income - cost
            
            # 计算性价比(搬迁方案实行后的收益 - 不实行搬迁方案的收益) / 搬迁过程中的投入成本
            # 避免除以零
            if cost > 0:
                cost_effectiveness = (income - baseline_income) / cost
            else:
                # 如果成本为0，则认为投入产出比无穷大（如果有收益增加）或0（如果没有收益增加）
                cost_effectiveness = float('inf') if income > baseline_income else 0
            
            fitness_values.append((cost_effectiveness, income, cost, profit))
        
        # 更新最佳解 - 以性价比为主要指标
        for i, (cost_effectiveness, income, cost, profit) in enumerate(fitness_values):
            if cost_effectiveness > best_cost_effectiveness:
                best_solution = population[i]
                best_income = income
                best_cost = cost
                best_profit = profit
                best_cost_effectiveness = cost_effectiveness
        
        # 进度报告
        if gen % 10 == 0 or gen == generations - 1:
            print(f"代数 {gen+1}/{generations} - "
                 f"当前最佳: 收入={best_income:.2f}, 成本={best_cost:.2f}, 利润={best_profit:.2f}, 性价比={best_cost_effectiveness:.4f} - "
                 f"耗时: {time.time() - start_time:.2f}秒")
            
            # 如果有最佳解，显示搬迁比例
            if best_solution:
                move_count = sum(1 for id_val, new_id in best_solution if id_val != new_id)
                move_percent = move_count / len(best_solution) * 100
                print(f"  搬迁居民: {move_count}/{len(best_solution)} ({move_percent:.2f}%)")
        
        # 如果达到最后一代，跳出循环
        if gen == generations - 1:
            break
        
        # 选择下一代
        new_population = []
        
        # 精英保留策略 - 保留最好的解决方案
        elite_count = max(1, int(population_size * elite_ratio))
        elite_indices = sorted(range(len(fitness_values)), 
                              key=lambda i: fitness_values[i][0],  # 按性价比排序
                              reverse=True)[:elite_count]
        
        for idx in elite_indices:
            new_population.append(population[idx])
        
        # 生成其余解决方案
        while len(new_population) < population_size:
            # 选择父代（锦标赛选择）
            tournament_size = 3
            p1_idx = max(random.sample(range(len(population)), tournament_size), 
                        key=lambda i: fitness_values[i][0])  # 按性价比选择
            p2_idx = max(random.sample(range(len(population)), tournament_size), 
                        key=lambda i: fitness_values[i][0])  # 按性价比选择
            
            parent1 = population[p1_idx]
            parent2 = population[p2_idx]
            
            # 交叉和变异
            child = crossover(parent1, parent2)
            child = repair_solution(child, occupied_plots)  # 确保解决方案有效
            child = mutate(child, data_df, mutation_rate=0.1)
            child = repair_solution(child, occupied_plots)  # 再次确保解决方案有效
            
            new_population.append(child)
            
        # 在生成新种群后
        for i in range(len(new_population)):
            new_population[i] = ensure_correct_solution(new_population[i], occupied_plots)
                
            # 在每次生成新种群后添加额外检查
        for i in range(len(new_population)):
            solution = new_population[i]
            if len(solution) != len(occupied_plots):
                print(f"警告: 第{i}个解决方案长度不等于{len(occupied_plots)}，实际长度为{len(solution)}")
                    
                # 修复解决方案，确保只包含有住户的地块
                solution_dict = {id_val: new_id for id_val, new_id in solution}
                correct_solution = [(id_val, solution_dict.get(id_val, id_val)) for id_val in occupied_plots_list]
                new_population[i] = correct_solution
        population = new_population
    
    print(f"遗传算法完成，总耗时: {time.time() - start_time:.2f}秒")
    return best_solution, best_income, best_cost, best_profit, baseline_income, best_cost_effectiveness

def ensure_correct_solution(solution, occupied_plots):
    """
    确保解决方案只包含有住户地块的映射，且长度正好为113个
    """
    # 创建一个映射字典
    solution_dict = {id_val: new_id for id_val, new_id in solution}
    
    # 创建一个新的解决方案，只包含有住户地块
    occupied_plots_list = sorted(list(occupied_plots))
    correct_solution = []
    
    # 为每个有住户的地块分配目标地块
    for id_val in occupied_plots_list:
        if id_val in solution_dict:
            # 如果原始解决方案中包含此地块，使用其目标地块
            correct_solution.append((id_val, solution_dict[id_val]))
        else:
            # 否则，默认原地不动
            correct_solution.append((id_val, id_val))
    
    # 确保没有冲突的目标地块
    assigned_targets = set()
    for i, (id_val, new_id) in enumerate(correct_solution):
        if new_id in assigned_targets:
            # 如果目标地块已被分配，改为原地不动
            if id_val not in assigned_targets:
                correct_solution[i] = (id_val, id_val)
                assigned_targets.add(id_val)
            else:
                # 寻找一个未分配的目标地块
                for candidate in range(1, 485):
                    if candidate not in assigned_targets:
                        correct_solution[i] = (id_val, candidate)
                        assigned_targets.add(candidate)
                        break
        else:
            assigned_targets.add(new_id)
    
    # 确保长度正好为113
    assert len(correct_solution) == len(occupied_plots), f"修复后的解决方案长度不等于{len(occupied_plots)}"
    
    return correct_solution

def main():
    try:
        # 加载数据
        print("开始加载数据...")
        data_df, adjoin_df, plots_df = load_data()
        
        # 打印数据统计
        id_counts = data_df['Id'].value_counts()
        print(f"不同Id的数量: {len(id_counts)}")
        print(f"映射选项总数: {len(data_df)}")
        
        # 分析数据集中不搬迁的比例
        stay_count = sum(1 for _, row in data_df.iterrows() if row['Id'] == row['newId'])
        print(f"原地不动的选项: {stay_count}/{len(data_df)} ({stay_count/len(data_df)*100:.2f}%)")
        
        # 数据预处理
        print("正在预处理数据...")
        block_df, yard_adjoins, block_id_to_index, yard_to_blocks = preprocess_data(plots_df, adjoin_df)
        
        # 使用遗传算法优化
        print("开始遗传算法优化...")
        best_solution, best_income, best_cost, best_profit, baseline_income, best_cost_effectiveness = genetic_algorithm(
            data_df, block_df, yard_adjoins, block_id_to_index, yard_to_blocks,
            population_size=100, generations=200
        )
        
        # 计算最佳解的详细指标
        if best_solution:
            print("\n========== 最佳搬迁方案分析 ==========")
            
            # 计算搬迁比例
            move_count = sum(1 for id_val, new_id in best_solution if id_val != new_id)
            total = len(best_solution)
            print(f"搬迁居民比例: {move_count}/{total} ({move_count/total*100:.2f}%)")
            
            # 计算空置院落和最终状态
            income, yard_empty, final_occupancy = calculate_income(
                best_solution, block_df, yard_adjoins, block_id_to_index, yard_to_blocks
            )
            
            # 计算空置院落总面积
            empty_yards_area, empty_yards = calculate_empty_yards_area(yard_empty, block_df)
            
            # 输出结果
            print("\n========== 搬迁方案经济分析 ==========")
            print(f"不实行搬迁方案的基准收益: {baseline_income:.2f}元")
            print(f"完全空置的院落数量: {len(empty_yards)}")
            print(f"完全空置的院落ID: {sorted(empty_yards)}")
            print(f"完全空置院落的总面积: {empty_yards_area:.2f}平方米")
            print(f"总投入成本: {best_cost:.2f}元")
            print(f"  - 其中修缮费: {best_cost - move_count * 30000:.2f}元")
            print(f"  - 其中沟通费: {move_count * 30000:.2f}元")
            print(f"搬迁方案实行后的收益: {best_income:.2f}元")
            print(f"净利润: {best_profit:.2f}元")
            print(f"性价比(m): {best_cost_effectiveness:.4f}")
            
            # 写入输出文件
            with open('问题三搬迁方案.csv', 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(['Id', 'newId'])
                for id_val, new_id in best_solution:
                    writer.writerow([id_val, new_id])
            
            print("最佳方案已保存到问题三搬迁方案.csv")
            
        
        else:
            print("未找到有效的搬迁方案")
    
    except Exception as e:
        print(f"程序运行出错: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
