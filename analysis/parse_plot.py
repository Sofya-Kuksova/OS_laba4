import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import re
import numpy as np

# Функция для парсинга одного лог-файла
# (оставляем без изменений)
def parse_log_file(log_file, process_type):
    data = []
    intervals = []
    current_start = None
    current_page = None
    with open(log_file, 'r') as f:
        for line in f:
            match = re.match(r'\[(\d+) ms\] (.+)', line)
            if match:
                time_ms, state_str = match.groups()
                time_ms = int(time_ms)
                state = state_str.strip()
                page = None
                if "page" in state:
                    page_match = re.search(r'page (\d+)', state)
                    if page_match:
                        page = int(page_match.group(1))
                        state = re.sub(r' page \d+', '', state)
                data.append({'time_ms': time_ms, 'state': state, 'page': page})

                if "START" in state:
                    current_start = time_ms
                    current_page = page
                elif "END" in state and current_start is not None:
                    intervals.append({
                        'start_time': current_start,
                        'end_time': time_ms,
                        'page': current_page,
                        'process_type': process_type
                    })
                    current_start = None
                    current_page = None
    df = pd.DataFrame(data)
    df['process_id'] = log_file.split('_')[-1].split('.')[0]
    df['process_type'] = process_type
    return df, pd.DataFrame(intervals)

# Функция для объединения данных из всех лог-файлов
# (оставляем без изменений)
def combine_logs(reader_logs, writer_logs):
    all_data = []
    all_intervals = []
    for log in reader_logs:
        df, intervals = parse_log_file(log, 'reader')
        all_data.append(df)
        all_intervals.append(intervals)
    for log in writer_logs:
        df, intervals = parse_log_file(log, 'writer')
        all_data.append(df)
        all_intervals.append(intervals)
    df = pd.concat(all_data).sort_values('time_ms').reset_index(drop=True)
    intervals = pd.concat(all_intervals).reset_index(drop=True)
    return df, intervals

# Функция для создания данных о доступе к страницам для heatmap
# (оставляем без изменений)
def create_page_access_df(intervals, time_bins, num_pages=12):
    page_access = []
    step = time_bins[1] - time_bins[0]
    for bin_start in time_bins[:-1]:
        bin_end = bin_start + step
        overlapping = intervals[(intervals['start_time_s'] < bin_end) & (intervals['end_time_s'] > bin_start)]
        for page in range(num_pages):
            page_overlaps = overlapping[overlapping['page'] == page]
            if not page_overlaps.empty:
                if 'writer' in page_overlaps['process_type'].values:
                    access_type = 'writer'
                else:
                    access_type = 'reader'
            else:
                access_type = 'none'
            page_access.append({'time_bin': bin_start, 'page': page, 'access_type': access_type})
    return pd.DataFrame(page_access)

# Функция для построения heatmap занятости страниц
# (оставляем без изменений)
def plot_page_occupancy_heatmap(page_access_df):
    heatmap_data = page_access_df.pivot(index='page', columns='time_bin', values='access_type')
    access_map = {'none': 0, 'reader': 1, 'writer': 2}
    heatmap_data_num = heatmap_data.map(lambda x: access_map[x])

    plt.figure(figsize=(15, 8))
    heatmap = sns.heatmap(heatmap_data_num, cmap='viridis', cbar_kws={'ticks': [0, 1, 2]})
    cbar = heatmap.collections[0].colorbar
    cbar.set_ticks([0, 1, 2])
    cbar.set_ticklabels(['none', 'reader', 'writer'])
    cbar.set_label('Тип доступа')
    plt.title('Занятость страниц во времени')
    plt.xlabel('Время (с)')
    plt.ylabel('Страница')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig('figures/heatmap.png')
    plt.close()

# Построение индивидуальных графиков состояний
def plot_individual_states(df):
    for (ptype, pid), group in df.groupby(['process_type', 'process_id']):
        grp = group.sort_values('time_s')
        states = list(dict.fromkeys(grp['state']))
        state_to_code = {state: idx for idx, state in enumerate(states)}
        codes = grp['state'].map(state_to_code)

        plt.figure(figsize=(10, 4))
        plt.step(grp['time_s'], codes, where='post')
        plt.yticks(list(state_to_code.values()), list(state_to_code.keys()))
        plt.xlabel('Время (с)')
        plt.ylabel('Состояние')
        plt.title(f'Смена состояний процесса {ptype} {pid}')
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f'figures/state_{ptype}_{pid}.png')
        plt.close()

def plot_all_processes_states(df):
    # Отдельно собираем список состояний каждого типа
    reader_states = list(dict.fromkeys(
        df[df['process_type']=='reader']['state']
    ))
    writer_states = list(dict.fromkeys(
        df[df['process_type']=='writer']['state']
    ))

    # Приводим reader-состояния к отрицательным кодам, writer — к положительным
    reader_code = {s: -(i+1) for i,s in enumerate(reader_states)}
    writer_code = {s: (i+1) for i,s in enumerate(writer_states)}

    readers = [(ptype, pid) for (ptype, pid), _ in df.groupby(['process_type','process_id']) if ptype=='reader']
    writers = [(ptype, pid) for (ptype, pid), _ in df.groupby(['process_type','process_id']) if ptype=='writer']
    reader_colors = plt.cm.Reds(np.linspace(0.4, 0.8, len(readers)))
    writer_colors = plt.cm.Blues(np.linspace(0.4, 0.8, len(writers)))

    plt.figure(figsize=(14, 8))
    for (ptype, pid), group in df.groupby(['process_type', 'process_id']):
        grp = group.sort_values('time_s')
        # Для каждого события подставляем код из нужного словаря
        if ptype == 'reader':
            codes = grp['state'].map(reader_code)
            color = reader_colors[readers.index((ptype, pid))]
        else:
            codes = grp['state'].map(writer_code)
            color = writer_colors[writers.index((ptype, pid))]

        label = f"{ptype}_{pid}"
        plt.step(grp['time_s'], codes, where='post', label=label, color=color)

    # Объединяем метки по обеим шкалам
    yticks = list(reader_code.values()) + list(writer_code.values())
    ylabels = reader_states + writer_states
    plt.yticks(yticks, ylabels)

    plt.xlabel('Время (с)')
    plt.ylabel('Состояние')
    plt.title('Смена состояний всех процессов')
    plt.legend(loc='upper right', bbox_to_anchor=(1.15, 1.0))
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('figures/state_all_processes.png')
    plt.close()


if __name__ == "__main__":
    # Параметры: число читателей и писателей
    N_READERS = 6
    N_WRITERS = 6
    reader_logs = [f'logs/reader_{i}.log' for i in range(1, N_READERS+1)]
    writer_logs = [f'logs/writer_{i}.log' for i in range(1, N_WRITERS+1)]

    # Объединяем данные из логов
    df, intervals = combine_logs(reader_logs, writer_logs)

    # Преобразуем время в секунды относительно минимального значения
    min_time = df['time_ms'].min()
    df['time_s'] = (df['time_ms'] - min_time) / 1000.0
    intervals['start_time_s'] = (intervals['start_time'] - min_time) / 1000.0
    intervals['end_time_s'] = (intervals['end_time'] - min_time) / 1000.0

    # Построение и сохранение тепмапы
    # Создаем временные интервалы для heatmap (шаг 0.1 с)
    max_time_s = df['time_s'].max()
    step = 0.5
    time_bins = np.arange(0, max_time_s + step, step)
    page_access_df = create_page_access_df(intervals, time_bins)
    plot_page_occupancy_heatmap(page_access_df)

    # Создание и сохранение индивидуальных графиков
    plot_individual_states(df)

    # Общий график состояний
    plot_all_processes_states(df)
