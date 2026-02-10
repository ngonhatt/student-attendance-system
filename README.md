# 📚 Student Attendance System using RFID

## 📌 Overview
This project implements a **Student Attendance System** using RFID technology.
Students scan their RFID cards to record attendance automatically.
The system displays information on an LCD and stores attendance data for later processing.

This project is developed as a **course project**.

---

## 🛠️ Hardware Components
- ESP32 Development Board
- RFID RC522 Module
- LCD 2004 (I2C)
- RTC DS1307
- RFID Cards
- Power Supply & Connecting Wires

---

## ⚙️ System Features
- Student identification using RFID cards
- Automatic attendance recording
- Real-time clock for date & time
- Display status and information on LCD
- Data storage for attendance tracking
- Export attendance data (Excel / Google Sheets – future work)

---

## 🧠 System Architecture
- ESP32 handles RFID scanning and logic
- RTC provides real-time timestamp
- LCD displays system status
- Attendance data is stored and processed

---

## 📷 Demo
* STUDENT ATTENDANCE SYSTEM PROTOTYPE
  
<img width="938" height="1424" alt="image" src="https://github.com/user-attachments/assets/77df1ead-a708-44c6-bf2f-a4aa19fe791d" />


* SCHEMATIC DIAGRAM

<img width="1231" height="705" alt="image" src="https://github.com/user-attachments/assets/46d5fc8c-c2b1-474b-9d3a-c19fa6ac8cb4" />


---

## 🧠 What I Learned
- Understanding the working principle of RFID-based attendance systems
- Interfacing RFID RC522 with ESP32
- Reading and handling UID data from RFID cards
- Using RTC module to obtain real-time date and time
- Displaying system information on LCD 2004 via I2C
- Designing system logic for student attendance management
- Structuring an embedded system project and documenting it on GitHub

---

## 🚀 Future Improvements
- Add a database to store student and attendance information
- Export attendance data to Excel or Google Sheets
- Develop a web-based interface for attendance management
- Add authentication and role management (admin / user)
- Improve system reliability and error handling
- Integrate cloud services for remote data access

---

## 📂 Project Structure
```text
student-attendance-system/
├── attendance_system.ino   # Main ESP32 program
├── README.md               # Project documentation
