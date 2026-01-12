# 🚗 Autonomous Smart Car Robot

## 📌 Giới thiệu
Dự án **Xe tự hành thông minh** sử dụng vi điều khiển (ESP32/Arduino) với nhiều chế độ hoạt động khác nhau như:
- Tự động tránh vật cản
- Điều khiển từ xa thông qua giao diện Web
- Dò line tự động

Dự án hướng tới việc ứng dụng IoT và Robot trong thực tế, phù hợp cho sinh viên ngành CNTT, Điện – Điện tử, Tự động hóa.

---

## ⚙️ Chức năng chính

### 🔹 1. Tự động tránh vật cản
- Sử dụng cảm biến siêu âm (HC-SR04)
- Xe tự động phát hiện chướng ngại vật phía trước
- Thực hiện các hành động: dừng – rẽ trái/phải – tiếp tục di chuyển

### 🔹 2. Điều khiển qua Web
- ESP32 tạo Web Server nội bộ
- Điều khiển xe bằng trình duyệt (điện thoại / máy tính)
- Các chức năng:
  - Tiến / Lùi / Trái / Phải
  - Dừng xe
  - Chuyển chế độ hoạt động

### 🔹 3. Dò line tự động
- Sử dụng cảm biến dò line hồng ngoại
- Xe có khả năng bám theo vạch đen trên nền trắng
- Áp dụng thuật toán điều khiển đơn giản (if–else)

---

## 🧰 Phần cứng sử dụng
- ESP32 / Arduino Uno
- Module điều khiển động cơ L298N
- Động cơ DC + bánh xe
- Cảm biến siêu âm HC-SR04
- Cảm biến dò line hồng ngoại
- Pin / Nguồn cấp
- Khung xe robot

---

## 💻 Phần mềm & Công nghệ
- Ngôn ngữ: **C/C++ (Arduino IDE)**
- Giao tiếp Web: **HTML, CSS**
- Giao thức: **WiFi (ESP32 Web Server)**
- Môi trường lập trình: **Arduino IDE**
