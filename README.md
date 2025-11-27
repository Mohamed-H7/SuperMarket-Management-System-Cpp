# SuperMarketPro

**SuperMarketPro** is a comprehensive desktop application developed as a university project using **C++** and the **Qt Framework**. The application aims to facilitate supermarket management operations, providing a robust interface for cashiers to process sales and a powerful dashboard for administrators to manage inventory, analyze data, and control user access.

## Project Objectives

* **Operational Efficiency:** Streamline the sales process to reduce human error and save time for cashiers.
* **Inventory Control:** Manage products effectively to prevent stock shortages or overstocking through real-time tracking.
* **Data Analysis:** Provide detailed reports and visual charts (Pie & Bar) to help managers make informed decisions based on profit and sales data.
* **Security:** Ensure secure access to the system through encrypted authentication and role-based access control (Admin/Cashier).

## Application Features

### 🔐 Authentication & Security
* **Secure Login:** Uses **SHA-256** algorithm to encrypt passwords before storing them in the database, ensuring high security.
* **User Management:** Admins can activate or deactivate accounts via the Control Panel; inactive accounts are blocked from logging in.

### 🛒 Store & Inventory Management
* **Product Operations:** Add, update, delete, and search for products by barcode or name.
* **Category Filtering:** Filter products by categories (e.g., Beverages, Snacks) for easier management.
* **Stock Alerts:** Real-time notifications for low-stock items (e.g., "This product is about to run out").

### 💻 Point of Sale (POS) Interface
* **Efficient Processing:** Cashiers can input product codes manually or via barcode simulation.
* **Dual Screen Support:** Features a "Free Window" button that creates a secondary screen for customers to view their shopping cart in real-time.
* **Smart Calculation:** Automatically calculates totals and applies active discounts.

### 📊 Reports & Analytics
* **Sales Reports:** View total sales, total profit, and product counts based on custom date ranges (Weekly, Monthly, Custom).
* **Visual Charts:**
    * **Pie Chart:** Shows profit percentage per category.
    * **Bar Chart:** Displays monthly sales distribution over the last 6 months.
* **Key Metrics:** Identifies best-selling products, least-selling products, and most profitable items.

### 🏷️ Discount System
* **Advanced Discounts:** Create discounts applied to specific products or entire categories.
* **Time-Based:** Discounts have start and end dates and are automatically validated during checkout.

### ⚙️ Settings & Localization
* **Profile Management:** Users can update their contact info and change passwords.
* **Multi-Language Support:** The application supports both **Turkish** and **English** languages.

## Tools & Technologies Used

* **Programming Language:** C++
* **Framework:** Qt (Qt Creator)
* **Database:** SQLite (SuperMarketPro_DataBase.db)
* **Visualization:** Qt Charts Library
* **Encryption:** SHA-256 Algorithm
* **IDE:** Qt Creator

---
_This project was developed by Mohamed Hamdo as part of the Computer Engineering coursework at Necmettin Erbakan University._
