#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <curl/curl.h>
#include <cstdlib>

// Encryption key (shift amount for Caesar Cipher)
const int SHIFT = 3;

// Function to split a string by a delimiter
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to convert an integer to a string (replacement for std::to_string)
std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Custom function to convert a string to an integer
int stringToInt(const std::string &str) {
    int result;
    std::istringstream(str) >> result;
    return result;
}

// Function to encrypt a string using Caesar Cipher
std::string encrypt(const std::string &text, int shift) {
    std::string result = text;
    for (size_t i = 0; i < result.size(); ++i) {
        if (isalpha(result[i])) {
            char base = islower(result[i]) ? 'a' : 'A';
            result[i] = (result[i] - base + shift) % 26 + base;
        }
    }
    return result;
}

// Function to decrypt a string using Caesar Cipher
std::string decrypt(const std::string &text, int shift) {
    return encrypt(text, 26 - shift);
}

// User class to represent each user
class User {
public:
    std::string role;
    std::string username;
    std::string password;

    User(std::string r, std::string u, std::string p) : role(r), username(u), password(p) {}

    bool operator<(const User &other) const {
        return username < other.username;
    }

    bool operator==(const User &other) const {
        return username == other.username;
    }
};

// UserNode class to represent each node in the binary search tree
class UserNode {
public:
    User user;
    UserNode* left;
    UserNode* right;

    UserNode(User u) : user(u), left(NULL), right(NULL) {}
};

// UserTree class to manage the binary search tree operations
class UserTree {
private:
    UserNode* root;

    UserNode* insert(UserNode* node, User user) {
        if (node == NULL) {
            return new UserNode(user);
        }
        if (user < node->user) {
            node->left = insert(node->left, user);
        } else {
            node->right = insert(node->right, user);
        }
        return node;
    }

    UserNode* findMin(UserNode* node) {
        while (node->left != NULL) node = node->left;
        return node;
    }

    UserNode* remove(UserNode* node, std::string username) {
        if (node == NULL) return node;
        if (username < node->user.username) {
            node->left = remove(node->left, username);
        } else if (username > node->user.username) {
            node->right = remove(node->right, username);
        } else {
            if (node->left == NULL) {
                UserNode* temp = node->right;
                delete node;
                return temp;
            } else if (node->right == NULL) {
                UserNode* temp = node->left;
                delete node;
                return temp;
            }
            UserNode* temp = findMin(node->right);
            node->user = temp->user;
            node->right = remove(node->right, temp->user.username);
        }
        return node;
    }

    UserNode* search(UserNode* node, std::string username) {
        if (node == NULL || node->user.username == username) {
            return node;
        }
        if (username < node->user.username) {
            return search(node->left, username);
        }
        return search(node->right, username);
    }

public:
    UserTree() : root(NULL) {}

    void insert(User user) {
        root = insert(root, user);
    }

    void remove(std::string username) {
        root = remove(root, username);
    }

    User* search(std::string username) {
        UserNode* result = search(root, username);
        if (result != NULL) {
            return &result->user;
        }
        return NULL;
    }

    void inOrder(UserNode* node, std::ofstream& file) {
        if (node != NULL) {
            inOrder(node->left, file);
            file << "Role:" << node->user.role << ";username:" << node->user.username << ";password:" << node->user.password << ",";
            inOrder(node->right, file);
        }
    }

    void saveToFile(std::string filename) {
        std::ofstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Error al abrir el archivo " << filename << std::endl;
            return;
        }
        inOrder(root, file);
        file.close();
    }

    UserNode* getRoot() {
        return root;
    }
};

// Function to load users from the file into the tree
UserTree loadUsers(const std::string &filename) {
    UserTree userTree;
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo " << filename << std::endl;
        return userTree;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> credentials = split(line, ',');
        for (size_t i = 0; i < credentials.size(); ++i) {
            std::vector<std::string> tokens = split(credentials[i], ';');
            if (tokens.size() >= 3) {
                std::string role = tokens[0].substr(tokens[0].find(":") + 1);
                std::string username = tokens[1].substr(tokens[1].find(":") + 1);
                std::string password = tokens[2].substr(tokens[2].find(":") + 1);
                userTree.insert(User(role, username, password));
            }
        }
    }
    file.close();
    return userTree;
}

// Function to validate user credentials
bool validateCredentials(UserTree &userTree, const std::string &username, const std::string &password) {
    User* user = userTree.search(username);
    if (user != NULL && user->password == password) {
        return true;
    }
    return false;
}

// Function to create a new user
void createUser(UserTree &userTree) {
    std::string username, password, role;
    std::cout << "Ingrese el nombre de usuario nuevo: ";
    std::cin >> username;
    std::cout << "Ingrese la contrasena nueva: ";
    std::cin >> password;
    std::cout << "Ingrese el rol (Admin/User): ";
    std::cin >> role;

    userTree.insert(User(role, username, password));
    std::cout << "Usuario creado exitosamente." << std::endl;
}

// Function to delete a user
void deleteUser(UserTree &userTree) {
    std::string username;
    std::cout << "Ingrese el nombre de usuario a eliminar: ";
    std::cin >> username;

    userTree.remove(username);
    std::cout << "Usuario eliminado exitosamente." << std::endl;
}

// Function to modify a user
void modifyUser(UserTree &userTree) {
    std::string username, newUsername, newPassword, newRole;
    std::cout << "Ingrese el nombre de usuario a modificar: ";
    std::cin >> username;

    User* user = userTree.search(username);
    if (user == NULL) {
        std::cout << "Usuario no encontrado." << std::endl;
        return;
    }

    std::cout << "Ingrese el nuevo nombre de usuario: ";
    std::cin >> newUsername;
    std::cout << "Ingrese la nueva contrasena: ";
    std::cin >> newPassword;
    std::cout << "Ingrese el nuevo rol (Admin/User): ";
    std::cin >> newRole;

    userTree.remove(username);  // Remove old user
    userTree.insert(User(newRole, newUsername, newPassword));  // Insert new user
    std::cout << "Usuario modificado exitosamente." << std::endl;
}

// Function to read the translation history into a map
std::map<std::string, int> readHistoryMap(const std::string &username) {
    std::string filename = "C:/Users/HOGAR/Desktop/5TO/PROGRA/proyecto/" + username + ".mydata";
    std::ifstream file(filename.c_str(), std::ios_base::binary);
    std::map<std::string, int> historyMap;

    if (file.is_open()) {
        std::string encryptedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::istringstream iss(encryptedData);
        std::string word;
        while (std::getline(iss, word, ',')) {
            if (!word.empty()) {
                std::string decryptedWord = decrypt(word, SHIFT);
                historyMap[decryptedWord]++;
            }
        }
        file.close();
    }

    return historyMap;
}

// Function to log the translation request to a user's file
void logTranslation(const std::string &username, const std::string &text) {
    std::string filename = "C:/Users/HOGAR/Desktop/5TO/PROGRA/proyecto/" + username + ".mydata";
    std::ofstream file;
    file.open(filename.c_str(), std::ios_base::binary | std::ios_base::app); // Open file in binary append mode
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo " << filename << std::endl;
        return;
    }

    std::string encryptedText = encrypt(text, SHIFT);
    file << encryptedText << ",";
    file.close();
}

// Function to read the translation history from a user's file and display it
void readTranslationHistory(const std::string &username) {
    std::map<std::string, int> historyMap = readHistoryMap(username);

    if (historyMap.empty()) {
        std::cout << "No se encontro historial de traduccion para el usuario " << username << "." << std::endl;
    } else {
        std::cout << "Historial de traduccion para el usuario " << username << ":" << std::endl;
        for (std::map<std::string, int>::iterator it = historyMap.begin(); it != historyMap.end(); ++it) {
            std::cout << it->first << " (" << it->second << ")" << std::endl;
        }
    }
}

// Function to capture the response data from the HTTP request
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

int main() {
    std::string username;
    std::string password;

    std::cout << "Ingrese su nombre de usuario: ";
    std::getline(std::cin, username);
    std::cout << "Ingrese su contrasena: ";
    std::getline(std::cin, password);

    UserTree userTree = loadUsers("C:/Users/HOGAR/Desktop/5TO/PROGRA/proyecto/usuarios.txt");

    if (!validateCredentials(userTree, username, password)) {
        std::cerr << "Nombre de usuario o contrasena invalidos. Saliendo..." << std::endl;
        return 1;
    }

    bool isAdmin = username == "admin";  // Simple check for admin user
    char choice;

    do {
        std::cout << "\nMenu:\n1. Traducir texto\n2. Ver historial de traduccion\n";
        if (isAdmin) {
            std::cout << "3. Crear usuario\n4. Eliminar usuario\n5. Modificar usuario\n";
        }
        std::cout << "x. Salir\nIngrese su eleccion: ";
        std::cin >> choice;

        switch(choice) {
            case '1': {
                std::cin.ignore(); // Clear input buffer
                // Variables to store user input
                std::string text;
                std::string source_language_code;
                std::string target_language_code;

                // Get input from the user
                std::cout << "Ingrese el texto a traducir: ";
                std::getline(std::cin, text);
                std::cout << "Ingrese el codigo del idioma de origen (por ejemplo, 'es' para espanol): ";
                std::getline(std::cin, source_language_code);
                std::cout << "Ingrese el codigo del idioma de destino (por ejemplo, 'en' para ingles): ";
                std::getline(std::cin, target_language_code);

                // Create a libcurl easy handle for the request
                CURL *curl = curl_easy_init();

                // Check if the handle was created successfully
                if (curl) {
                    // Set the URL for the Google Translate API endpoint
                    curl_easy_setopt(curl, CURLOPT_URL, "https://translation.googleapis.com/language/translate/v2");
                    curl_easy_setopt(curl, CURLOPT_POST, 1);

                    // Construct the request body in JSON format
                    std::string body = "{";
                    body += "\"q\": \"" + text + "\",";
                    body += "\"source\": \"" + source_language_code + "\",";
                    body += "\"target\": \"" + target_language_code + "\"";
                    body += "}";

                    // Set the request body
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

                    // Set the request headers
                    struct curl_slist *headers = NULL;
                    headers = curl_slist_append(headers, "X-Goog-Api-Key: AIzaSyAUoEPIcTqW4D9vTJFnwlAF0_WeZuI0pWM");
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                    // Set up a custom write function to capture the response data
                    std::string response;
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

                    // Disable SSL certificate verification for simplicity (not recommended for production)
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

                    // Perform the HTTP request
                    CURLcode res = curl_easy_perform(curl);

                    // Check if the request was successful and extract the translated text
                    if (res == CURLE_OK) {
                        // Find the index of the start and end of the translated text
                        size_t start = response.find("\"translatedText\":") + 19; // 17 is the length of "\"translatedText\": \""
                        size_t end = response.find("\"\n", start);
                        
                        
                       /* std::cout << "response lenght " << response.length() << std::endl;
                        std::cout << "start " << start << std::endl;
                        std::cout << "end " << end << std::endl;
                        */
                        
                        // Extract the translated text substring
                         std::string translatedText = response.substr(start, end - start);
                        std::cout << "Texto traducido: " << translatedText << std::endl;
                        
                          // Log the translation request
                        logTranslation(username, text);
                        
                       
                         std::string comando = "espeak -v " + target_language_code + " \"" + translatedText + "\" --stdout > output.wav";

   						 // Llamar a espeak para generar el archivo de audio
   						 int resultado = system(comando.c_str());

 					    if (resultado == 0) {
     					   std::cout << "Audio generado exitosamente." << std::endl;
   						 } else {
     					   std::cerr << "Error al generar el audio." << std::endl;
   						 }
                        
                        
                       
                      
                    } else {
                        std::cerr << "Error al realizar la solicitud: " << curl_easy_strerror(res) << std::endl;
                    }

                    // Clean up libcurl resources
                    curl_easy_cleanup(curl);
                    curl_slist_free_all(headers);
                } else {
                    std::cerr << "Error al inicializar libcurl" << std::endl;
                }

                break;
            }
            case '2':
                // View Translation History logic
                readTranslationHistory(username);
                break;
            case '3':
                if (isAdmin) createUser(userTree);
                break;
            case '4':
                if (isAdmin) deleteUser(userTree);
                break;
            case '5':
                if (isAdmin) modifyUser(userTree);
                break;
            case 'x':
                std::cout << "Saliendo..." << std::endl;
                break;
            default:
                std::cerr << "Eleccion invalida. Por favor intente de nuevo." << std::endl;
        }
    } while (choice != 'x');

    userTree.saveToFile("C:/Users/HOGAR/Desktop/5TO/PROGRA/proyecto/usuarios.txt");

    return 0;
}

