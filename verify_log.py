import pandas as pd
import time

# ================= CẤU HÌNH =================
FILE_PATH = 'data/halo_dataset_1_5m.csv' # Đường dẫn tới file 1.5 triệu dòng của bạn

# Input cho User (Khớp với test case bạn vừa chạy)
TARGET_USER = 'U03649'
USER_START = 1717682115
USER_END = 9999999999

# Input cho Resource (Ví dụ lấy Top 1 lúc nãy)
TARGET_RESOURCE = 'R03922'
RES_START = 1713500000
RES_END = 1714500000
# ============================================

print(f"Đang nạp dữ liệu từ {FILE_PATH}...")
start_time = time.time()
try:
    df = pd.read_csv(FILE_PATH)
    print(f"Đã nạp {len(df)} dòng trong {time.time() - start_time:.2f} giây.\n")
except Exception as e:
    print(f"Lỗi đọc file: {e}")
    exit()

print(f"[HANH TRINH CUA {TARGET_USER}]")
# Lọc User và khoảng thời gian
user_df = df[(df['user_id'] == TARGET_USER) & (df['timestamp'] >= USER_START) & (df['timestamp'] <= USER_END)].copy()
# Sắp xếp tăng dần theo thời gian (giống Lazy Sort)
user_df = user_df.sort_values(by='timestamp')

count_u = 0
for index, row in user_df.iterrows():
    count_u += 1
    # In ra với format Y HỆT C++
    print(f"- Time {row['timestamp']}: {row['device_id']} -> {row['app_id']} -> {row['resource_id']}")
print(f"-> Tổng cộng: {count_u} hoạt động.\n")


print(f"[HANH TRINH TRUY XUAT TAI NGUYEN {TARGET_RESOURCE}]")
# Lọc Resource và khoảng thời gian
res_df = df[(df['resource_id'] == TARGET_RESOURCE) & (df['timestamp'] >= RES_START) & (df['timestamp'] <= RES_END)].copy()
# Sắp xếp tăng dần theo thời gian
res_df = res_df.sort_values(by='timestamp')

count_r = 0
for index, row in res_df.iterrows():
    count_r += 1
    # In ra với format Y HỆT C++
    print(f"- Time {row['timestamp']}: {row['user_id']} -> {row['device_id']} -> {row['app_id']}")
print(f"-> Tổng cộng: {count_r} hoạt động.")
