#include <mysql.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>
#ifdef _WIN32
#include <conio.h> 
#else
#include <termios.h>
#include <unistd.h>
#endif
using namespace std;
string getPassword() {
    string password;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {  
        if (ch == '\b') {  // Handle backspace
            if (!password.empty()) {
                cout << "\b \b";
                password = password.substr(0, password.size() - 1); 
            }
        } else {
            cout << '*';
            password += ch;
        }
    }
#else

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    getline(cin, password);  

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    cout << endl;  
    return password;
}
class User {
protected:
    string username;
    string password;
public:
    User(string uname, string pass) : username(uname), password(pass) {}
    virtual bool login(MYSQL* conn) = 0;
    virtual void displayActions(MYSQL* conn) = 0;
     void viewInventory(MYSQL* conn) {
        cout << "Displaying Inventory...\n";
        string query = "SELECT MED_ID, MED_NAME, MED_QTY, CATEGORY, MED_PRICE, LOCATION_RACK FROM meds";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                MYSQL_ROW row;
                cout << left << setw(10) << "ID" << setw(20) << "Name" 
                     << setw(10) << "Quantity" << setw(15) << "Category" 
                     << setw(10) << "Price" << setw(15) << "Location\n";
                cout << string(80, '-') << endl;

                while ((row = mysql_fetch_row(res))) {
                    cout << left << setw(10) << row[0] << setw(20) << row[1]
                         << setw(10) << row[2] << setw(15) << row[3]
                         << setw(10) << row[4] << setw(15) << row[5] << endl;
                }
                mysql_free_result(res);
            } else {
                cerr << "Failed to fetch inventory data: " << mysql_error(conn) << endl;
            }
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        }
    }
    void showCustomersMenu(MYSQL* conn) {
        int customerChoice;
        do {
            cout << "\n---Customers Management ---\n";
            cout << "1. View Customers\n";
            cout << "2. Add Customers\n";
            cout << "3. Update Customers\n";
            cout << "4. Delete Customers\n";
            cout << "5. Back to Admin Dashboard\n";
            cout << "Enter your choice: ";
            cin >> customerChoice;
            switch (customerChoice) {
                case 1: viewCustomer(conn); break;
                case 2: addCustomer(conn); break;
                case 3: updateCustomer(conn); break;
                case 4: deleteCustomer(conn); break;
                case 5: return;
                default: 
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (customerChoice != 5);
    }
    void viewCustomer(MYSQL* conn) {
    cout << "Displaying Customers...\n";
    string query = "SELECT C_ID, C_FNAME, C_LNAME, C_AGE, C_SEX, C_PHNO, C_MAIL FROM customer";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            cout << left << setw(10) << "ID" << setw(15) << "First Name" 
                 << setw(15) << "Last Name" << setw(5) << "Age" 
                 << setw(10) << "Sex" << setw(15) << "Phone" 
                 << setw(30) << "Email\n";
            cout << string(90, '-') << endl;

            while ((row = mysql_fetch_row(res))) {
                cout << left << setw(10) << row[0] << setw(15) << row[1]
                     << setw(15) << row[2] << setw(5) << row[3]
                     << setw(10) << row[4] << setw(15) << row[5]
                     << setw(30) << row[6] << endl;
            }
            mysql_free_result(res);
        } else {
            cerr << "Failed to fetch customer data: " << mysql_error(conn) << endl;
        }
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}
void addCustomer(MYSQL* conn) {
    string custId, firstName, lastName, age, sex, phone, email, username, password;    
    cout << "Enter Customer ID: ";
    cin >> custId;
    string checkQuery = "SELECT COUNT(*) FROM customer WHERE C_ID = '" + custId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) > 0) {
            cerr << "Error: Customer ID already exists. Cannot add duplicate customer ID.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter First Name: ";
    cin >> firstName;
    cout << "Enter Last Name: ";
    cin >> lastName;
    cout << "Enter Age: ";
    cin >> age;
    cout << "Enter Sex: ";
    cin >> sex;
    cout << "Enter Phone Number: ";
    cin >> phone;
    cout << "Enter Email: ";
    cin >> email;
    cout << "Enter Username: ";
    cin >> username;
    cout << "Enter Password: ";
    cin >> password;
    string insertQuery = "INSERT INTO customer (C_ID, C_FNAME, C_LNAME, C_AGE, C_SEX, C_PHNO, C_MAIL, C_USERNAME, C_PASSWORD) VALUES ('" 
                         + custId + "', '" + firstName + "', '" + lastName + "', '" + age + "', '" + sex + "', '" + phone + "', '" + email + "', '" + username + "', '" + password + "')";
    if (mysql_query(conn, insertQuery.c_str()) == 0) {
        cout << "Customer added successfully!\n";
    } else {
        cerr << "Failed to add customer: " << mysql_error(conn) << endl;
    }
}
void updateCustomer(MYSQL* conn) {
    string custId, firstName, lastName, age, sex, phone, email;
    cout << "Enter Customer ID to Update: ";
    cin >> custId;
    string checkQuery = "SELECT COUNT(*) FROM customer WHERE C_ID = '" + custId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Customer ID does not exist. Cannot update a non-existing customer.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter New First Name: ";
    cin >> firstName;
    cout << "Enter New Last Name: ";
    cin >> lastName;
    cout << "Enter New Age: ";
    cin >> age;
    cout << "Enter New Sex: ";
    cin >> sex;
    cout << "Enter New Phone Number: ";
    cin >> phone;
    cout << "Enter New Email: ";
    cin >> email;
    string updateQuery = "UPDATE customer SET C_FNAME = '" + firstName + "', C_LNAME = '" + lastName + "', C_AGE = '" + age + "', C_SEX = '" + sex + "', C_PHNO = '" + phone + "', C_MAIL = '" + email + "' WHERE C_ID = '" + custId + "'";
    if (mysql_query(conn, updateQuery.c_str()) == 0) {
        cout << "Customer updated successfully!\n";
    } else {
        cerr << "Failed to update customer: " << mysql_error(conn) << endl;
    }
}
void deleteCustomer(MYSQL* conn) {
    string custId;
    cout << "Enter Customer ID to Delete: ";
    cin >> custId;
    string checkQuery = "SELECT COUNT(*) FROM customer WHERE C_ID = '" + custId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Customer ID does not exist. Cannot delete a non-existing customer.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    string deleteQuery = "DELETE FROM customer WHERE C_ID = '" + custId + "'";
    if (mysql_query(conn, deleteQuery.c_str()) == 0) {
        cout << "Customer deleted successfully!\n";
    } else {
        cerr << "Failed to delete customer: " << mysql_error(conn) << endl;
    }
}
    void logout() {
        cout << "Logging out...\n";
    }
};
class Admin : public User {
public:
    Admin(string uname, string pass) : User(uname, pass) {}
    bool login(MYSQL* conn) override {
        string query = "SELECT COUNT(*) FROM admin WHERE A_USERNAME = '" + username + "' AND A_PASSWORD = '" + password + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row = mysql_fetch_row(res);

            bool loginSuccess = row && atoi(row[0]) > 0;
            mysql_free_result(res);
            return loginSuccess;
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
            return false;
        }
    }
    void displayActions(MYSQL* conn) override {
        int choice;
        do {
            cout << "\n--- Admin Dashboard ---\n";
            cout << "1. Inventory\n";
            cout << "2. Suppliers\n";
            cout << "3. Employees\n";
            cout << "4. Customers\n";
            cout << "5. View Sales Invoice Details\n";
            cout << "6. Medicines - Soon to Expire\n";
            cout << "7. Medicines - Low Stock\n";
            cout << "8. Logout\n";
            cout << "Enter your choice: ";
            cin >> choice;
            switch (choice) {
                case 1: 
                    showInventoryMenu(conn);
                    break;
                case 2: 
                    showSuppliersMenu(conn);
                    break;
                case 3: showEmployeesMenu(conn); break;
                case 4: showCustomersMenu(conn); break;
                case 5: showSalesInvoice(conn); break;
                case 6: medSoonToExpire(conn); break;
                case 7: medLowStock(conn); break;
                case 8: 
                    logout();
                    return;
                default: 
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (choice != 8);
    }
private:
    void showInventoryMenu(MYSQL* conn) {
        int inventoryChoice;
        do {
            cout << "\n--- Inventory Management ---\n";
            cout << "1. View Inventory\n";
            cout << "2. Add Medicines\n";
            cout << "3. Update Inventory\n";
            cout << "4. Delete Medicines\n";
            cout << "5. Back to Admin Dashboard\n";
            cout << "Enter your choice: ";
            cin >> inventoryChoice;
            switch (inventoryChoice) {
                case 1: viewInventory(conn); break;
                case 2: addMedicine(conn); break;
                case 3: updateInventory(conn); break;
                case 4: deleteMedicine(conn); break;
                case 5: return;
                default: 
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (inventoryChoice != 5);
    }
    void showSuppliersMenu(MYSQL* conn) {
        int supplierChoice;
        do {
            cout << "\n--- Supplier Management ---\n";
            cout << "1. View Suppliers\n";
            cout << "2. Add Supplier\n";
            cout << "3. Update Supplier\n";
            cout << "4. Delete Supplier\n";
            cout << "5. Back to Admin Dashboard\n";
            cout << "Enter your choice: ";
            cin >> supplierChoice;
            switch (supplierChoice) {
                case 1: viewSuppliers(conn); break;
                case 2: addSupplier(conn); break;
                case 3: updateSupplier(conn); break;
                case 4: deleteSupplier(conn); break;
                case 5: return;
                default: 
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (supplierChoice != 5);
    }
    void showEmployeesMenu(MYSQL* conn) {
        int employeeChoice;
        do {
            cout << "\n---Employee Management ---\n";
            cout << "1. View Employees\n";
            cout << "2. Add Employees\n";
            cout << "3. Update Employees\n";
            cout << "4. Delete Employees\n";
            cout << "5. Back to Admin Dashboard\n";
            cout << "Enter your choice: ";
            cin >> employeeChoice;
            switch (employeeChoice) {
                case 1: viewEmployee(conn); break;
                case 2: addEmployee(conn); break;
                case 3: updateEmployee(conn); break;
                case 4: deleteEmployee(conn); break;
                case 5: return;
                default: 
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (employeeChoice != 5);
    }
    void addMedicine(MYSQL* conn) {
        string medId, medName, category, locationRack;
        int medQty;
        double medPrice;

        while (true) {
            cout << "Enter Medicine ID: ";
            cin >> medId;
            string checkQuery = "SELECT COUNT(*) FROM meds WHERE MED_ID = '" + medId + "'";
            if (mysql_query(conn, checkQuery.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                MYSQL_ROW row = mysql_fetch_row(res);
                if (atoi(row[0]) > 0) {
                    cout << "Medicine ID already exists. Please enter a unique ID.\n";
                    mysql_free_result(res);
                    continue;  
                }
                mysql_free_result(res);
                break; 
            } else {
                cerr << "Query Execution Error: " << mysql_error(conn) << endl;
                return;
            }
        }
        cout << "Enter Medicine Name: ";
        cin.ignore();
        getline(cin, medName);
        cout << "Enter Category: ";
        getline(cin, category);
        cout << "Enter Quantity: ";
        cin >> medQty;
        cout << "Enter Price: ";
        cin >> medPrice;
        cout << "Enter Location Rack: ";
        cin.ignore();
        getline(cin, locationRack);        
        ostringstream qtyStream, priceStream;
        qtyStream << medQty;
        priceStream << medPrice;
        string insertQuery = "INSERT INTO meds (MED_ID, MED_NAME, MED_QTY, CATEGORY, MED_PRICE, LOCATION_RACK) VALUES ('" +
                             medId + "', '" + medName + "', " + qtyStream.str() + ", '" + category + "', " + priceStream.str() + ", '" + locationRack + "')";
        if (mysql_query(conn, insertQuery.c_str()) == 0) {
            cout << "Medicine added successfully!\n";
        } else {
            cerr << "Failed to add medicine: " << mysql_error(conn) << endl;
        }
    }
    void updateInventory(MYSQL* conn) {
        string medId, newMedName, newCategory, newLocationRack;
        int newMedQty;
        double newMedPrice;   
        cout << "Enter Medicine ID to update: ";
        cin >> medId; 
        string fetchQuery = "SELECT MED_NAME, MED_QTY, CATEGORY, MED_PRICE, LOCATION_RACK FROM meds WHERE MED_ID = '" + medId + "'";
        if (mysql_query(conn, fetchQuery.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) { 
                cout << "Current details of " << medId << ":\n";
                cout << "Name: " << row[0] << "\n";
                cout << "Quantity: " << row[1] << "\n";
                cout << "Category: " << row[2] << "\n";
                cout << "Price: " << row[3] << "\n";
                cout << "Location Rack: " << row[4] << "\n";
                mysql_free_result(res); 
                cout << "Enter new Medicine Name: ";
                cin.ignore();
                getline(cin, newMedName);
                if (newMedName.empty()) newMedName = row[0];  
                cout << "Enter new Quantity: ";
                cin >> newMedQty;
                if (newMedQty == 0) newMedQty = atoi(row[1]);  
                cout << "Enter new Category: ";
                cin.ignore();
                getline(cin, newCategory);
                if (newCategory.empty()) newCategory = row[2];  
                cout << "Enter new Price: ";
                cin >> newMedPrice;
                if (newMedPrice == 0) newMedPrice = atof(row[3]);  
                cout << "Enter new Location Rack: ";
                cin.ignore();
                getline(cin, newLocationRack);
                if (newLocationRack.empty()) newLocationRack = row[4]; 
                ostringstream qtyStream, priceStream;
                qtyStream << newMedQty;
                priceStream << newMedPrice;
                string updateQuery = "UPDATE meds SET MED_NAME = '" + newMedName + "', MED_QTY = " + qtyStream.str() + 
                                     ", CATEGORY = '" + newCategory + "', MED_PRICE = " + priceStream.str() + 
                                     ", LOCATION_RACK = '" + newLocationRack + "' WHERE MED_ID = '" + medId + "'";
                if (mysql_query(conn, updateQuery.c_str()) == 0) {
                    cout << "Medicine updated successfully!\n";
                } else {
                    cerr << "Failed to update medicine: " << mysql_error(conn) << endl;
                }
            } else {
                cerr << "Medicine not found with ID: " << medId << endl;
                mysql_free_result(res);
            }
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        }
    }
    void deleteMedicine(MYSQL* conn) {
        string medId;  
        while (true) {
            cout << "Enter Medicine ID to delete: ";
            cin >> medId;
            string checkQuery = "SELECT COUNT(*) FROM meds WHERE MED_ID = '" + medId + "'";
            if (mysql_query(conn, checkQuery.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row && atoi(row[0]) > 0) {
                    string deleteQuery = "DELETE FROM meds WHERE MED_ID = '" + medId + "'";
                    if (mysql_query(conn, deleteQuery.c_str()) == 0) {
                        cout << "Medicine with ID " << medId << " has been deleted successfully!\n";
                        mysql_free_result(res);
                        break;  
                    } else {
                        cerr << "Error deleting medicine: " << mysql_error(conn) << endl;
                        mysql_free_result(res);
                        break;
                    }
                } else {
                    cout << "Medicine with ID " << medId << " does not exist. Please enter a valid ID.\n";
                    mysql_free_result(res);
                }
            } else {
                cerr << "Query Execution Error: " << mysql_error(conn) << endl;
                break;
            }
        }
    }
    void viewSuppliers(MYSQL* conn) {
        cout << "Displaying Suppliers...\n";
        string query = "SELECT SUP_ID, SUP_NAME, SUP_ADD, SUP_PHNO, SUP_MAIL FROM suppliers";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                MYSQL_ROW row;
                cout << left << setw(10) << "ID" << setw(20) << "Name" 
                     << setw(30) << "Address" << setw(15) << "Phone" 
                     << setw(30) << "Email\n";
                cout << string(80, '-') << endl;

                while ((row = mysql_fetch_row(res))) {
                    cout << left << setw(10) << row[0] << setw(20) << row[1]
                         << setw(30) << row[2] << setw(15) << row[3]
                         << setw(30) << row[4] << endl;
                }
                mysql_free_result(res);
            } else {
                cerr << "Failed to fetch supplier data: " << mysql_error(conn) << endl;
            }
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        }
    }
void addSupplier(MYSQL* conn) {
    string supId, supName, supAddress, supPhone, supEmail;
    
    cout << "Enter Supplier ID: ";
    cin >> supId;
    string checkQuery = "SELECT COUNT(*) FROM suppliers WHERE SUP_ID = '" + supId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) > 0) {
            cerr << "Error: Supplier ID already exists. Cannot add duplicate supplier ID.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter Supplier Name: ";
    cin.ignore();
    getline(cin, supName);
    cout << "Enter Supplier Address: ";
    getline(cin, supAddress);
    cout << "Enter Supplier Phone: ";
    getline(cin, supPhone);
    cout << "Enter Supplier Email: ";
    getline(cin, supEmail);
    
    string insertQuery = "INSERT INTO suppliers (SUP_ID, SUP_NAME, SUP_ADD, SUP_PHNO, SUP_MAIL) VALUES ('" + supId + "', '" + supName + "', '" + supAddress + "', '" + supPhone + "', '" + supEmail + "')";
    if (mysql_query(conn, insertQuery.c_str()) == 0) {
        cout << "Supplier added successfully!\n";
    } else {
        cerr << "Failed to add supplier: " << mysql_error(conn) << endl;
    }
}
void updateSupplier(MYSQL* conn) {
    string supId, supName, supAddress, supPhone, supEmail;
    cout << "Enter Supplier ID to Update: ";
    cin >> supId;
    string checkQuery = "SELECT COUNT(*) FROM suppliers WHERE SUP_ID = '" + supId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Supplier ID does not exist. Cannot update a non-existing supplier.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter New Supplier Name: ";
    cin.ignore();
    getline(cin, supName);
    cout << "Enter New Supplier Address: ";
    getline(cin, supAddress);
    cout << "Enter New Supplier Phone: ";
    getline(cin, supPhone);
    cout << "Enter New Supplier Email: ";
    getline(cin, supEmail);
    string updateQuery = "UPDATE suppliers SET SUP_NAME = '" + supName + "', SUP_ADD = '" + supAddress + "', SUP_PHNO = '" + supPhone + "', SUP_MAIL = '" + supEmail + "' WHERE SUP_ID = '" + supId + "'";
    if (mysql_query(conn, updateQuery.c_str()) == 0) {
        cout << "Supplier updated successfully!\n";
    } else {
        cerr << "Failed to update supplier: " << mysql_error(conn) << endl;
    }
}
void deleteSupplier(MYSQL* conn) {
    string supId;
    cout << "Enter Supplier ID to Delete: ";
    cin >> supId;
    string checkQuery = "SELECT COUNT(*) FROM suppliers WHERE SUP_ID = '" + supId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Supplier ID does not exist. Cannot delete a non-existing supplier.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    string deleteQuery = "DELETE FROM suppliers WHERE SUP_ID = '" + supId + "'";
    if (mysql_query(conn, deleteQuery.c_str()) == 0) {
        cout << "Supplier deleted successfully!\n";
    } else {
        cerr << "Failed to delete supplier: " << mysql_error(conn) << endl;
    }
}
void viewEmployee(MYSQL* conn) {
    cout << "Displaying Employees...\n";
    string query = "SELECT E_ID, E_FNAME, E_LNAME, BDATE, E_AGE, E_SEX, E_TYPE, E_JDATE, E_SAL, E_PHNO, E_MAIL FROM employee";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            cout << left << setw(10) << "ID" << setw(20) << "First Name" << setw(20) << "Last Name" 
                 << setw(12) << "Birth Date" << setw(5) << "Age" << setw(10) << "Sex" << setw(15) << "Job Type" 
                 << setw(15) << "Join Date" << setw(10) << "Salary" << setw(15) << "Phone" << setw(30) << "Email\n";
            cout << string(150, '-') << endl;

            while ((row = mysql_fetch_row(res))) {
                cout << left << setw(10) << row[0] << setw(20) << row[1] << setw(20) << row[2]
                     << setw(12) << row[3] << setw(5) << row[4] << setw(10) << row[5] << setw(15) << row[6] 
                     << setw(15) << row[7] << setw(10) << row[8] << setw(15) << row[9] << setw(30) << row[10] << endl;
            }
            mysql_free_result(res);
        } else {
            cerr << "Failed to fetch employee data: " << mysql_error(conn) << endl;
        }
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}
void addEmployee(MYSQL* conn) {
    string empId, firstName, lastName, birthDate, age, sex, jobType, joinDate, salary, phone, email, address;
    cout << "Enter Employee ID: ";
    cin >> empId;
    string checkQuery = "SELECT COUNT(*) FROM employee WHERE E_ID = '" + empId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) > 0) {
            cerr << "Error: Employee ID already exists. Cannot add duplicate employee ID.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter First Name: ";
    cin.ignore();
    getline(cin, firstName);
    cout << "Enter Last Name: ";
    getline(cin, lastName);
    cout << "Enter Birth Date (YYYY-MM-DD): ";
    getline(cin, birthDate);
    cout << "Enter Age: ";
    getline(cin, age);
    cout << "Enter Sex: ";
    getline(cin, sex);
    cout << "Enter Job Type: ";
    getline(cin, jobType);
    cout << "Enter Join Date (YYYY-MM-DD): ";
    getline(cin, joinDate);
    cout << "Enter Salary: ";
    getline(cin, salary);
    cout << "Enter Phone Number: ";
    getline(cin, phone);
    cout << "Enter Email: ";
    getline(cin, email);
    cout << "Enter Address: ";
    getline(cin, address);
    string insertQuery = "INSERT INTO employee (E_ID, E_FNAME, E_LNAME, BDATE, E_AGE, E_SEX, E_TYPE, E_JDATE, E_SAL, E_PHNO, E_MAIL, E_ADD) "
                         "VALUES ('" + empId + "', '" + firstName + "', '" + lastName + "', '" + birthDate + "', '" + age + "', '" 
                         + sex + "', '" + jobType + "', '" + joinDate + "', '" + salary + "', '" + phone + "', '" + email + "', '" + address + "')";   
    if (mysql_query(conn, insertQuery.c_str()) == 0) {
        cout << "Employee added successfully!\n";
    } else {
        cerr << "Failed to add employee: " << mysql_error(conn) << endl;
    }
}
void updateEmployee(MYSQL* conn) {
    string empId, firstName, lastName, birthDate, age, sex, jobType, joinDate, salary, phone, email, address;
    cout << "Enter Employee ID to Update: ";
    cin >> empId;
    string checkQuery = "SELECT COUNT(*) FROM employee WHERE E_ID = '" + empId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Employee ID does not exist. Cannot update non-existing employee.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter New First Name: ";
    cin.ignore();
    getline(cin, firstName);
    cout << "Enter New Last Name: ";
    getline(cin, lastName);
    cout << "Enter New Birth Date (YYYY-MM-DD): ";
    getline(cin, birthDate);
    cout << "Enter New Age: ";
    getline(cin, age);
    cout << "Enter New Sex: ";
    getline(cin, sex);
    cout << "Enter New Job Type: ";
    getline(cin, jobType);
    cout << "Enter New Join Date (YYYY-MM-DD): ";
    getline(cin, joinDate);
    cout << "Enter New Salary: ";
    getline(cin, salary);
    cout << "Enter New Phone Number: ";
    getline(cin, phone);
    cout << "Enter New Email: ";
    getline(cin, email);
    cout << "Enter New Address: ";
    getline(cin, address);
    string updateQuery = "UPDATE employee SET E_FNAME = '" + firstName + "', E_LNAME = '" + lastName + "', BDATE = '" + birthDate + "', E_AGE = '" 
                         + age + "', E_SEX = '" + sex + "', E_TYPE = '" + jobType + "', E_JDATE = '" + joinDate + "', E_SAL = '"
                         + salary + "', E_PHNO = '" + phone + "', E_MAIL = '" + email + "', E_ADD = '" + address + "' WHERE E_ID = '" 
                         + empId + "'";
    
    if (mysql_query(conn, updateQuery.c_str()) == 0) {
        cout << "Employee updated successfully!\n";
    } else {
        cerr << "Failed to update employee: " << mysql_error(conn) << endl;
    }
}
void deleteEmployee(MYSQL* conn) {
    string empId;
    cout << "Enter Employee ID to Delete: ";
    cin >> empId;
    string checkQuery = "SELECT COUNT(*) FROM employee WHERE E_ID = '" + empId + "'";
    if (mysql_query(conn, checkQuery.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && atoi(row[0]) == 0) {
            cerr << "Error: Employee ID does not exist. Cannot delete non-existing employee.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    string deleteQuery = "DELETE FROM employee WHERE E_ID = '" + empId + "'";
    
    if (mysql_query(conn, deleteQuery.c_str()) == 0) {
        cout << "Employee deleted successfully!\n";
    } else {
        cerr << "Failed to delete employee: " << mysql_error(conn) << endl;
    }
}

void showSalesInvoice(MYSQL* conn) {
    std::cout << "Displaying Sales Invoices...\n";
    std::string query = "SELECT SALE_ID, C_ID, S_DATE, S_TIME, TOTAL_AMT, E_ID FROM sales";   
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            std::cout << std::left << std::setw(10) << "Sale ID " 
                      << std::setw(10) << "Customer_ID " 
                      << std::setw(15) << "Sale_Date " 
                      << std::setw(10) << "Sale_Time " 
                      << std::setw(12) << "Total_Amount " 
                      << std::setw(10) << "Employee_ID\n";
            std::cout << std::string(70, '-') << std::endl;
            while ((row = mysql_fetch_row(res))) {
                std::cout << std::left << std::setw(10) << row[0] 
                          << std::setw(10) << row[1] 
                          << std::setw(15) << row[2] 
                          << std::setw(10) << row[3] 
                          << std::setw(12) << row[4] 
                          << std::setw(10) << row[5] << std::endl;
            }
            mysql_free_result(res);
        } else {
            std::cerr << "Failed to fetch sales invoice data: " << mysql_error(conn) << std::endl;
        }
    } else {
        std::cerr << "Query Execution Error: " << mysql_error(conn) << std::endl;
    }
}
void medSoonToExpire(MYSQL* conn) {
    std::cout << "Displaying Medicines Soon to Expire (within 6 months)...\n";
    std::string query = "SELECT MED_ID, EXP_DATE FROM purchase WHERE EXP_DATE <= DATE_ADD(CURDATE(), INTERVAL 6 MONTH)";
    
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            std::cout << std::left << std::setw(10) << "Med ID" 
                      << std::setw(20) << "Expiry Date\n";
            std::cout << std::string(30, '-') << std::endl;

            while ((row = mysql_fetch_row(res))) {
                std::cout << std::left << std::setw(10) << row[0] 
                          << std::setw(20) << row[1] << std::endl;
            }
            mysql_free_result(res);
        } else {
            std::cerr << "Failed to fetch expiry data: " << mysql_error(conn) << std::endl;
        }
    } else {
        std::cerr << "Query Execution Error: " << mysql_error(conn) << std::endl;
    }
}
void medLowStock(MYSQL* conn) {
    std::cout << "Displaying Medicines with Low Stock (quantity < 50)...\n";
    std::string query = "SELECT MED_ID, MED_NAME, MED_QTY FROM meds WHERE MED_QTY < 50";
    
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            std::cout << std::left << std::setw(10) << "Med ID" 
                      << std::setw(20) << "Medicine Name" 
                      << std::setw(10) << "Quantity\n";
            std::cout << std::string(40, '-') << std::endl;

            while ((row = mysql_fetch_row(res))) {
                std::cout << std::left << std::setw(10) << row[0] 
                          << std::setw(20) << row[1] 
                          << std::setw(10) << row[2] << std::endl;
            }
            mysql_free_result(res);
        } else {
            std::cerr << "Failed to fetch low stock data: " << mysql_error(conn) << std::endl;
        }
    } else {
        std::cerr << "Query Execution Error: " << mysql_error(conn) << std::endl;
    }
}

};
class Pharmacist : public User {
private:
    int employeeId; 
public:
    Pharmacist(string uname, string pass) : User(uname, pass), employeeId(0) {}
    bool login(MYSQL* conn) override {
        string query = "SELECT E_ID FROM emplogin WHERE E_USERNAME = '" + username + "' AND E_PASS = '" + password + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                employeeId = atoi(row[0]);  
                mysql_free_result(res);
                return true;  
            }
            mysql_free_result(res);
            return false; 
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
            return false;
        }
    }
    int getEmployeeId() const {
        return employeeId;  
    }
    void displayActions(MYSQL* conn) override{
        int choice;
        do {
            cout << "\n--- Pharmacist Dashboard ---\n";
            cout << "1. View Inventory\n";
            cout << "2. Add Sales\n";
            cout << "3. Customer Details\n";
            cout << "4. Logout\n";
            cout << "Enter your choice: ";
            cin >> choice;
            switch (choice) {
                case 1:
                    viewInventory(conn);
                    break;
                case 2:
                    addSales(conn);
                    break;
                case 3:
                    showCustomersMenu(conn);
                    break;
                case 4:
                    logout();
                    return;
                default:
                    cout << "Invalid choice. Please try again.\n";
                    break;
            }
        } while (choice != 4);
    }
void addSales(MYSQL* conn) {
    cout << "Enter Customer ID: ";
    int customerId;
    cin >> customerId;
    ostringstream oss;
    oss << customerId;
    string customerIdStr = oss.str();
    string query = "SELECT * FROM customer WHERE C_ID = '" + customerIdStr + "'";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            cout << "Customer not found.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter Medicine Name: ";
    string medName;
    cin >> ws;
    getline(cin, medName);
    query = "SELECT MED_ID, MED_NAME, MED_QTY, MED_PRICE, CATEGORY FROM meds WHERE MED_NAME = '" + medName + "'";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            cout << "Medicine not found.\n";
            mysql_free_result(res);
            return;
        }
        string medId = row[0];
        string medName = row[1];
        int availableQty = atoi(row[2]);
        double price = atof(row[3]);
        string category = row[4];        
        cout << "Medicine Details:\n";
        cout << "Name: " << medName << "\n";
        cout << "Category: " << category << "\n";
        cout << "Available Quantity: " << availableQty << "\n";
        cout << "Price per Unit: " << fixed << setprecision(2) << price << "\n";
        mysql_free_result(res);        
        cout << "Enter quantity required: ";
        int qty;
        cin >> qty;       
        if (qty > availableQty) {
            cout << "Not enough stock available.\n";
            return;
        }
        double totalPrice = qty * price;
        cout << "Total Price: " << fixed << setprecision(2) << totalPrice << "\n";
        time_t now = time(0);
        struct tm* currentTime = localtime(&now);
        char date[20], time[10];
        strftime(date, sizeof(date), "%Y-%m-%d", currentTime);
        strftime(time, sizeof(time), "%H:%M:%S", currentTime);
        ostringstream totalPriceStream;
        totalPriceStream << totalPrice;
        string totalPriceStr = totalPriceStream.str();
        ostringstream employeeIdStream;
        employeeIdStream << getEmployeeId();
        string employeeIdStr = employeeIdStream.str();
        query = "INSERT INTO sales (C_ID, S_DATE, S_TIME, TOTAL_AMT, E_ID) VALUES ('" + customerIdStr + "', '" + string(date) + "', '" + string(time) + "', '" + totalPriceStr + "', '" + employeeIdStr + "')";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Sale recorded successfully.\n";
        } else {
            cerr << "Error recording sale: " << mysql_error(conn) << endl;
            return;
        }
        int saleId = mysql_insert_id(conn);
        oss.str(""); oss << saleId;  
        string saleIdStr = oss.str();       
        oss.str(""); oss << qty;  
        string qtyStr = oss.str();       
        oss.str(""); oss << totalPrice;  
        string totalPriceItemStr = oss.str();        
        query = "INSERT INTO sales_items (SALE_ID, MED_ID, SALE_QTY, TOT_PRICE) VALUES ('" + saleIdStr + "', '" + medId + "', '" + qtyStr + "', '" + totalPriceItemStr + "')";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Sale item added successfully.\n";
        } else {
            cerr << "Error adding sale item: " << mysql_error(conn) << endl;
            return;
        }
        query = "UPDATE meds SET MED_QTY = MED_QTY - " + qtyStr + " WHERE MED_ID = '" + medId + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Medicine stock updated.\n";
        } else {
            cerr << "Error updating medicine stock: " << mysql_error(conn) << endl;
            return;
        }
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}
};
class Customer : public User {
private:
    int customerId;	
public:
    Customer(string uname, string pass) : User(uname, pass), customerId(-1) {}
    int getCustomerId() {
        return customerId;
    }
	bool login(MYSQL* conn) override {
        string query = "SELECT C_ID FROM customer WHERE C_USERNAME = '" + username + "' AND C_PASSWORD = '" + password + "'";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                customerId = atoi(row[0]);
                mysql_free_result(res);
                return true;
            } else {
                mysql_free_result(res);
                return false;
            }
        } else {
            cerr << "Query Execution Error: " << mysql_error(conn) << endl;
            return false;
        }
    }
    void displayActions(MYSQL* conn) override {
    int choice;
    do {
        cout << "\n--- Customer Dashboard ---\n";
        cout << "1. Profile\n"; 
        cout << "2. Make Order\n"; 
        cout << "3. Order Histories\n"; 
        cout << "4. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                profile(conn);  
                break;
            case 2:
                makeOrder(conn); 
                break;
            case 3:
                orderHistory(conn); 
                break;
            case 4:
                logout(); 
                return;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    } while (choice != 4);
}
void profile(MYSQL* conn) {
    string query = "SELECT C_FNAME, C_LNAME, C_AGE, C_SEX, C_PHNO, C_MAIL FROM customer WHERE C_USERNAME = '" + username + "'";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            cout << "\n--- Customer Profile ---\n";
            cout << "First Name: " << row[0] << endl;
            cout << "Last Name: " << row[1] << endl;
            cout << "Age: " << row[2] << endl;
            cout << "Sex: " << row[3] << endl;
            cout << "Phone Number: " << row[4] << endl;
            cout << "Email: " << (row[5] ? row[5] : "N/A") << endl;
        } else {
            cout << "No customer details found.\n";
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}
void makeOrder(MYSQL* conn) {
    ostringstream customerIdStream;
    customerIdStream << getCustomerId();  
    string customerIdStr = customerIdStream.str();
    string query = "SELECT * FROM customer WHERE C_ID = '" + customerIdStr + "'";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            cout << "Customer not found.\n";
            mysql_free_result(res);
            return;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
        return;
    }
    cout << "Enter Medicine Name: ";
    string medName;
    cin >> ws;
    getline(cin, medName);
    query = "SELECT MED_ID, MED_NAME, MED_QTY, MED_PRICE, CATEGORY FROM meds WHERE MED_NAME = '" + medName + "'";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            cout << "Medicine not found.\n";
            mysql_free_result(res);
            return;
        }
        string medId = row[0];
        string medName = row[1];
        int availableQty = atoi(row[2]);
        double price = atof(row[3]);
        string category = row[4];
        cout << "Medicine Details:\n";
        cout << "Name: " << medName << "\n";
        cout << "Category: " << category << "\n";
        cout << "Available Quantity: " << availableQty << "\n";
        cout << "Price per Unit: " << fixed << setprecision(2) << price << "\n";
        mysql_free_result(res);
        cout << "Enter quantity required: ";
        int qty;
        cin >> qty;
        if (qty > availableQty) {
            cout << "Not enough stock available.\n";
            return;
        }
        double totalPrice = qty * price;
        cout << "Total Price: " << fixed << setprecision(2) << totalPrice << "\n";

        // Show options to pay or cancel
        cout << "Choose an option:\n1. Pay\n2. Cancel\n";
        int choice;
        cin >> choice;

        if (choice == 2) {
            cout << "Order cancelled.\n";
            return;
        } else if (choice == 1) {
            // Process payment
            time_t now = time(0);
            struct tm* currentTime = localtime(&now);
            char date[20], time[10];
            strftime(date, sizeof(date), "%Y-%m-%d", currentTime);
            strftime(time, sizeof(time), "%H:%M:%S", currentTime);

            ostringstream totalPriceStream;
            totalPriceStream << totalPrice;
            string totalPriceStr = totalPriceStream.str();
            string employeeIdStr = "1"; 
            query = "INSERT INTO sales (C_ID, S_DATE, S_TIME, TOTAL_AMT, E_ID) VALUES ('" + customerIdStr + "', '" + string(date) + "', '" + string(time) + "', '" + totalPriceStr + "', '" + employeeIdStr + "')";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Sale recorded successfully.\n";
            } else {
                cerr << "Error recording sale: " << mysql_error(conn) << endl;
                return;
            }
            int saleId = mysql_insert_id(conn); 
            ostringstream saleIdStream;
            saleIdStream << saleId;
            string saleIdStr = saleIdStream.str();
            ostringstream qtyStream;
            qtyStream << qty;
            string qtyStr = qtyStream.str();
            ostringstream totalPriceItemStream;
            totalPriceItemStream << totalPrice;
            string totalPriceItemStr = totalPriceItemStream.str();
            query = "INSERT INTO sales_items (SALE_ID, MED_ID, SALE_QTY, TOT_PRICE) VALUES ('" + saleIdStr + "', '" + medId + "', '" + qtyStr + "', '" + totalPriceItemStr + "')";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Sale item added successfully.\n";
            } else {
                cerr << "Error adding sale item: " << mysql_error(conn) << endl;
                return;
            }
            query = "UPDATE meds SET MED_QTY = MED_QTY - " + qtyStr + " WHERE MED_ID = '" + medId + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Medicine stock updated.\n";
            } else {
                cerr << "Error updating medicine stock: " << mysql_error(conn) << endl;
                return;
            }
            cout << "Payment successful. Thank you for your purchase!\n";
        } else {
            cout << "Invalid choice. Order cancelled.\n";
            return;
        }
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}

void orderHistory(MYSQL* conn) {
    if (customerId == -1) {
        cout << "Customer is not logged in.\n";
        return;
    }
    ostringstream customerIdStream;
    customerIdStream << customerId;
    string customerIdStr = customerIdStream.str();
    string query = "SELECT si.MED_ID, m.MED_NAME, si.SALE_QTY, si.TOT_PRICE, s.S_DATE "
                   "FROM sales s "
                   "JOIN sales_items si ON s.SALE_ID = si.SALE_ID "
                   "JOIN meds m ON si.MED_ID = m.MED_ID "
                   "WHERE s.C_ID = '" + customerIdStr + "' "
                   "ORDER BY s.S_DATE DESC";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;       
        cout << "\n--- Order History ---\n";
        cout << left << setw(10) << "Med ID" 
             << left << setw(30) << "Medicine Name" 
             << left << setw(10) << "Quantity" 
             << left << setw(15) << "Total Price" 
             << left << setw(15) << "Date" << endl;
        while ((row = mysql_fetch_row(res))) {
            cout << left << setw(10) << row[0]   
                 << left << setw(30) << row[1]  
                 << left << setw(10) << row[2]   
                 << left << setw(15) << fixed << setprecision(2) << atof(row[3])  
                 << left << setw(15) << row[4]   
                 << endl;
        }
        mysql_free_result(res);
    } else {
        cerr << "Query Execution Error: " << mysql_error(conn) << endl;
    }
}
};
int main() {
    MYSQL* conn;
    string server = "localhost", user = "root", password = "pms123", database = "pharmacy";
    conn = mysql_init(0);
    if (conn) {
        cout << "MySQL Library initialized.\n";
    } else {
        cerr << "MySQL Initialization failed.\n";
        return 1;
    }
    conn = mysql_real_connect(conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 3306, NULL, 0);
    if (conn) {
        cout << "Connection established.\n";
    } else {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        return 1;
    }
    int roleChoice;
    string username, pass;
    User* userPtr = NULL; 
    while (true) {
        cout << "Select login role:\n1. Admin\n2. Pharmacist\n3. Customer\nEnter choice: ";
        cin >> roleChoice;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        pass = getPassword();  
        delete userPtr;  
        if (roleChoice == 1) {
            userPtr = new Admin(username, pass);
        } else if (roleChoice == 2) {
            userPtr = new Pharmacist(username, pass);
        } else if (roleChoice == 3) {
            userPtr = new Customer(username, pass);
        } else {
            cout << "Invalid role choice. Please try again.\n";
            continue;
        }
        if (userPtr->login(conn)) {
            cout << "Login successful!\n";
            userPtr->displayActions(conn);
            delete userPtr;  
            break;
        } else {
            cout << "Invalid username or password. Please try again.\n";
            delete userPtr;  
            userPtr = NULL;
        }
    }
    mysql_close(conn);
    return 0;
}

