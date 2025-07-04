#pragma once



class LanguageManager {
public:
    std::unordered_map<std::string, std::string> lang_en = {};

    std::unordered_map<std::string, std::string> lang_es = {
        {"No open files yet!", "Aun no hay archivos abiertos!"},
        {"Drag a file onto the window to get started", "Arrastre un archivo a la ventana para comenzar"},
        {"Data Viewer", "Visor de datos"},
        {"CPK Viewer", "Visor CPK"},
        {"Configuration", "Ajustes"},
        {"Automatically Load Textures", "Cargar texturas automaticamente"},
        {"Show All SCR Meshes By Default", "Mostrar todos los modelos SCR por defecto"},
        {"Select CPK Directory", "Seleccione el directorio CPK"},
        {"Save", "Guardar"},
        {"Skeleton", "Esqueleto"},
        {"Tools", "Herramientas"},
        {"Materials", "Materials"}
    };


    std::unordered_map<std::string, std::string>* currentLang = &lang_en;
    // Access the one and only instance
    static LanguageManager& Instance();

    void Init();
    const char* TR(const std::string& key) {
        auto it = currentLang->find(key);
        return (it != currentLang->end()) ? it->second.c_str() : key.c_str();
    }

    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;
    LanguageManager(LanguageManager&&) = delete;
    LanguageManager& operator=(LanguageManager&&) = delete;
private:



    LanguageManager();

    ~LanguageManager();
};