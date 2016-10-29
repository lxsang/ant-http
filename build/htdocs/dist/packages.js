(function() {
  window.OSjs = window.OSjs || {}
  OSjs.Core = OSjs.Core || {}
  OSjs.Core.getMetadata = function() {
    return Object.freeze({
    "default/About": {
        "className": "ApplicationAbout",
        "name": "About OS.js",
        "description": "About OS.js",
        "names": {
            "bg_BG": " За OS.js",
            "de_DE": "Über OS.js",
            "fr_FR": "À propos d'OS.js",
            "it_IT": "Informazioni su OS.js",
            "ko_KR": "OS.js에 대하여",
            "nl_NL": "Over OS.js",
            "no_NO": "Om OS.js",
            "pl_PL": "o OS.js",
            "ru_RU": "Об OS.js",
            "sk_SK": "o OS.js",
            "tr_TR": "hakkında OS.js",
            "vi_VN": "Thông tin về OS.js"
        },
        "descriptions": {
            "bg_BG": "За OS.js",
            "de_DE": "Über OS.js",
            "fr_FR": "À propos d'OS.js",
            "it_IT": "Informazioni su OS.js",
            "ko_KR": "OS.js에 대하여",
            "nl_NL": "Over OS.js",
            "no_NO": "Om OS.js",
            "pl_PL": "o OS.js",
            "ru_RU": "Об OS.js",
            "sk_SK": "o OS.js",
            "tr_TR": "hakkında OS.js",
            "vi_VN": "Thông tin về OS.js"
        },
        "singular": true,
        "category": "system",
        "icon": "apps/help-browser.png",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/About",
        "build": {},
        "repo": "default"
    },
    "default/Calculator": {
        "className": "ApplicationCalculator",
        "name": "Calculator",
        "names": {
            "bg_Bg": "Клакулатор",
            "fr_FR": "Calculatrice",
            "it_IT": "Calcolatrice",
            "ko_KR": "계산기",
            "nl_NL": "Rekenmachine",
            "no_NO": "Kalkulator",
            "pl_PL": "Kalkulator",
            "ru_RU": "Калькулятор",
            "sk_SK": "Kalkulačka",
            "tr_TR": "Hesap Makinesi",
            "vi_VN": "Máy tính"
        },
        "icon": "apps/calc.png",
        "category": "office",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Calculator",
        "build": {},
        "repo": "default"
    },
    "default/CoreWM": {
        "className": "CoreWM",
        "name": "OS.js Window Manager",
        "names": {
            "bg_BG": "Мениджър на прозорци на OS.js",
            "de_DE": "OS.js Fenster-Manager",
            "es_ES": "OS.js Window Manager",
            "fr_FR": "Gestionnaire de fenêtre OS.js",
            "it_IT": "OS.js Gestore Finestre",
            "ko_KR": "OS.js 윈도우 관리자",
            "nl_NL": "OS.js venster beheer",
            "no_NO": "OS.js Vinduhåndterer",
            "pl_PL": "Menedżer Okien OS.js",
            "ru_RU": "OS.js Оконный менеджер",
            "sk_SK": "Správca Okien OS.js",
            "tr_TR": "OS.js Pencere Yöneticisi",
            "vi_VN": "Quản lí cửa sổ OS.js"
        },
        "singular": true,
        "type": "windowmanager",
        "icon": "apps/gnome-window-manager.png",
        "splash": false,
        "preload": [
            {
                "src": "scheme.html",
                "type": "scheme"
            },
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            }
        ],
        "panelItems": {
            "AppMenu": {
                "Name": "AppMenu",
                "Description": "Application Menu",
                "Icon": "actions/stock_about.png",
                "HasOptions": false
            },
            "Buttons": {
                "Name": "Buttons",
                "Description": "Button Bar",
                "Icon": "actions/stock_about.png"
            },
            "Clock": {
                "Name": "Clock",
                "Description": "View the time",
                "Icon": "status/appointment-soon.png",
                "HasOptions": true
            },
            "NotificationArea": {
                "Name": "NotificationArea",
                "Description": "View notifications",
                "Icon": "apps/gnome-panel-notification-area.png"
            },
            "Search": {
                "Name": "Search",
                "Description": "Perform searches",
                "Icon": "actions/find.png",
                "HasOptions": true
            },
            "Weather": {
                "Name": "Weather",
                "Description": "Weather notification",
                "Icon": "status/weather-few-clouds.png"
            },
            "WindowList": {
                "Name": "Window List",
                "Description": "Toggle between open windows",
                "Icon": "apps/xfwm4.png"
            }
        },
        "path": "default/CoreWM",
        "build": {},
        "repo": "default"
    },
    "default/Draw": {
        "className": "ApplicationDraw",
        "name": "Draw",
        "description": "Simple drawing application",
        "names": {
            "bg_BG": "Рисуване",
            "de_DE": "Zeichnen",
            "fr_FR": "Dessin",
            "it_IT": "Disegna",
            "ko_KR": "그림판",
            "nl_NL": "Tekenen",
            "no_NO": "Tegne",
            "pl_PL": "Rysowanie",
            "ru_RU": "Графический редактор",
            "sk_SK": "Kreslenie",
            "tr_TR": "Çiz",
            "vi_VN": "Vẽ"
        },
        "descriptions": {
            "bg_BG": "Приложение за рисуване",
            "de_DE": "Einfaches Zeichen-Programm",
            "fr_FR": "Programme de dessin simple",
            "it_IT": "Semplice applicazione per creazione/modifica immagini",
            "ko_KR": "간단한 그리기 응용 프로그램",
            "nl_NL": "Eenvoudig tekenprogramma",
            "no_NO": "Simpelt tegne-program",
            "pl_PL": "Prosta aplikacja do rysowania",
            "ru_RU": "Простой графический редактор",
            "sk_SK": "Jednoduchá aplikácia na kreslenie",
            "vi_VN": "Phần mềm vẽ đơn giản"
        },
        "mime": [
            "^image"
        ],
        "category": "graphics",
        "icon": "categories/gnome-graphics.png",
        "compability": [
            "canvas"
        ],
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Draw",
        "build": {},
        "repo": "default"
    },
    "default/FileManager": {
        "className": "ApplicationFileManager",
        "name": "File Manager",
        "description": "The default file manager",
        "names": {
            "bg_BG": "Файлов мениджър",
            "de_DE": "Dateimanager",
            "fr_FR": "Explorateur de fichier",
            "it_IT": "Gestore File",
            "nl_NL": "bestands beheer",
            "no_NO": "Fil-håndtering",
            "pl_PL": "Menedżer Plików",
            "ko_KR": "파일 탐색기",
            "sk_SK": "Správca súborov",
            "ru_RU": "Файловый менеджер",
            "tr_TR": "Dosya Yöneticisi",
            "vi_VN": "Quản lí file"
        },
        "descriptions": {
            "bg_BG": "Стандартния файлов мениджър",
            "de_DE": "Standardmäßiger Dateimanager",
            "fr_FR": "Gestionnaire de fichier par défaut",
            "it_IT": "Il gestore file predefinito",
            "nl_NL": "Standaard bestands beheerder",
            "no_NO": "Standard Fil-håndtering program",
            "pl_PL": "Domyślny Menedżer Plików",
            "ko_KR": "기본 파일 관리자",
            "sk_SK": "Štandardný správca súborov",
            "ru_RU": "Стандартный файловый менеджер",
            "tr_TR": "Varsayılan dosya yöneticisi",
            "vi_VN": "Trình quản lí file mặc định"
        },
        "category": "utilities",
        "icon": "apps/file-manager.png",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/FileManager",
        "build": {},
        "repo": "default"
    },
    "default/HTMLViewer": {
        "className": "ApplicationHTMLViewer",
        "name": "HTML Viewer",
        "mime": [
            "text\\/html"
        ],
        "icon": "mimetypes/html.png",
        "category": "utilities",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/HTMLViewer",
        "build": {},
        "repo": "default"
    },
    "default/MusicPlayer": {
        "className": "ApplicationMusicPlayer",
        "name": "Music Player",
        "names": {
            "bg_BG": "Музикален плеър",
            "de_DE": "Musikspieler",
            "es_ES": "Music Player",
            "fr_FR": "Lecteur de musique",
            "it_IT": "Lettore Musicale",
            "ko_KR": "뮤직 플레이어",
            "nl_NL": "Audio speler",
            "no_NO": "Musikkspiller",
            "pl_PL": "Odtwarzacz muzyki",
            "ru_RU": "Аудиоплеер",
            "sk_SK": "Prehrávač hudby",
            "tr_TR": "Muzik Çalar",
            "vi_VN": "Nghe nhạc"
        },
        "mime": [
            "^audio",
            "osjs\\/playlist"
        ],
        "category": "multimedia",
        "icon": "status/audio-volume-high.png",
        "singular": true,
        "compability": [
            "audio"
        ],
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/MusicPlayer",
        "build": {},
        "repo": "default"
    },
    "default/Preview": {
        "className": "ApplicationPreview",
        "name": "Preview",
        "description": "Preview image files",
        "names": {
            "bg_BG": "Преглед на изображения",
            "de_DE": "Vorschau",
            "fr_FR": "Visionneuse",
            "it_IT": "Anteprima Immagini",
            "ko_KR": "미리보기",
            "nl_NL": "Foto viewer",
            "no_NO": "Forhåndsviser",
            "pl_PL": "Podgląd",
            "ru_RU": "Просмотрщик",
            "sk_SK": "Prehliadač obrázkov",
            "tr_TR": "Önizle",
            "vi_VN": "Trình xem ảnh"
        },
        "descriptions": {
            "bg_BG": "Преглед на изображения",
            "de_DE": "Bildervorschau",
            "fr_FR": "Visionneuse de photos",
            "it_IT": "Anteprima Immagini",
            "ko_KR": "이미지 파일을 미리 봅니다",
            "nl_NL": "Foto viewer",
            "no_NO": "Forhåndsvisning av bilde-filer",
            "pl_PL": "Podgląd zdjęć",
            "ru_RU": "Просмотрщик изображений",
            "sk_SK": "Prehliadač obrázkov",
            "tr_TR": "resim dosyalarını önizle",
            "vi_VN": "Trình xem ảnh"
        },
        "mime": [
            "^image",
            "^video"
        ],
        "category": "multimedia",
        "icon": "mimetypes/image.png",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Preview",
        "build": {},
        "repo": "default"
    },
    "default/ProcessViewer": {
        "className": "ApplicationProcessViewer",
        "name": "Process Viewer",
        "description": "View running processes",
        "names": {
            "bg_BG": "Процеси",
            "de_DE": "Prozess-Manager",
            "fr_FR": "Gestionnaire de processus",
            "it_IT": "Gestore Attività",
            "ko_KR": "프로세스 관리자",
            "nl_NL": "Proces manager",
            "no_NO": "Prosess oversikt",
            "pl_PL": "Procesy",
            "ru_RU": "Менеджер процессов",
            "sk_SK": "Správca procesov",
            "tr_TR": "İşlemleri Görüntüle",
            "vi_VN": "Xem tiến trình"
        },
        "descriptions": {
            "bg_BG": "Преглед на процеси",
            "de_DE": "Laufende Prozesse verwalten",
            "fr_FR": "Visualiser les processus en cours",
            "it_IT": "Mostri processi attivi",
            "ko_KR": "실행 중인 프로세스를 관리합니다",
            "nl_NL": "Bekijk de lopende processen",
            "no_NO": "Se oversikt over kjørende prosesser",
            "pl_PL": "Zobacz działające procesy",
            "ru_RU": "Менеджер запущенных процессов",
            "sk_SK": "Spravovanie bežiacich procesov",
            "tr_TR": "çalışan işlemleri görüntüle",
            "vi_VN": "Xem các tiến trình đang chạy"
        },
        "singular": true,
        "category": "system",
        "icon": "apps/gnome-monitor.png",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/ProcessViewer",
        "build": {},
        "repo": "default"
    },
    "default/Settings": {
        "className": "ApplicationSettings",
        "preloadParallel": true,
        "name": "Settings",
        "mime": null,
        "icon": "categories/applications-system.png",
        "category": "system",
        "singular": true,
        "names": {
            "bg_BG": "Настройки",
            "de_DE": "Einstellungen",
            "es_ES": "Settings",
            "fr_FR": "Paramètres",
            "it_IT": "Settaggi",
            "ko_KR": "환경설정",
            "nl_NL": "Instellingen",
            "no_NO": "Instillinger",
            "pl_PL": "Ustawienia",
            "ru_RU": "Настройки",
            "sk_SK": "Nastavenia",
            "tr_TR": "Ayarlar",
            "vi_VN": "Cài đặt"
        },
        "descriptions": {
            "bg_BG": "Настройки",
            "de_DE": "Einstellungen",
            "es_ES": "Settings",
            "fr_FR": "Paramètres",
            "it_IT": "Settaggi",
            "ko_KR": "환경설정",
            "nl_NL": "Instellingen",
            "no_NO": "Instillinger",
            "pl_PL": "Ustawienia",
            "ru_RU": "Настройки",
            "sk_SK": "Nastavenia",
            "tr_TR": "Program Ayarlarını düzenle",
            "vi_VN": "Cài đặt"
        },
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Settings",
        "build": {},
        "repo": "default"
    },
    "default/Textpad": {
        "className": "ApplicationTextpad",
        "name": "Textpad",
        "description": "Simple text editor",
        "names": {
            "bg_BG": "Текстов редактор",
            "de_DE": "Texteditor",
            "fr_FR": "Éditeur de texte",
            "it_IT": "Editor Testi",
            "ko_KR": "텍스트패드",
            "nl_NL": "Notities",
            "no_NO": "Tekstblokk",
            "pl_PL": "Notatnik",
            "ru_RU": "Редактор текста",
            "sk_SK": "Poznámkový blok",
            "tr_TR": "Basit Bir Metin Düzenleyicisi",
            "vi_VN": "Trình sửa văn bản"
        },
        "descriptions": {
            "bg_BG": "Стандартен текстов редактор",
            "de_DE": "Einfacher Texteditor",
            "fr_FR": "Éditeur de texte simple",
            "it_IT": "Semplice editor di testi",
            "ko_KR": "간단한 텍스트 편집기",
            "nl_NL": "Eenvoudige Tekstverwerker",
            "no_NO": "Simpel tekst redigering",
            "pl_PL": "Prosty edytor tekstu",
            "ru_RU": "Простой текстовый редактор",
            "sk_SK": "Jednoduchý textový editor",
            "tr_TR": "Basit Bir Metin Düzenleyicisi",
            "vi_VN": "Trình sửa văn bản đơn giản"
        },
        "mime": [
            "^text",
            "inode\\/x\\-empty",
            "application\\/x\\-empty",
            "application\\/x\\-lua",
            "application\\/x\\-python",
            "application\\/javascript",
            "application\\/json"
        ],
        "category": "utilities",
        "icon": "apps/accessories-text-editor.png",
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Textpad",
        "build": {},
        "repo": "default"
    },
    "default/Writer": {
        "className": "ApplicationWriter",
        "name": "Writer",
        "description": "Write rich text documents",
        "names": {
            "bg_BG": "Текст",
            "de_DE": "Writer",
            "fr_FR": "Traitement de texte",
            "it_IT": "Editor Testi",
            "ko_KR": "글쓰기",
            "nl_NL": "Tekstverwerker",
            "no_NO": "Writer",
            "pl_PL": "Writer",
            "ru_RU": "Текстовый процессор",
            "sk_SK": "Writer",
            "tr_TR": "Writer",
            "vi_VN": "Viết"
        },
        "descriptions": {
            "bg_BG": "Писанене на обогатен текст",
            "de_DE": "Verfassen Sie Rich-Text-Dokumente",
            "fr_FR": "Traitement de texte riche",
            "it_IT": "Scrivi documenti di testo formattati",
            "ko_KR": "리치 텍스트 문서 작성",
            "nl_NL": "Tekst verwerker met opmaak",
            "no_NO": "Skriv riktekst dokumenter",
            "pl_PL": "Twórz dokumenty rich text",
            "ru_RU": "Текстовый процессор",
            "sk_SK": "Editor na tvorbu Rich-Text dokumentov",
            "tr_TR": "Gelişmiş dökümanlar oluşturun",
            "vi_VN": "Xem, chỉnh sửa văn bản"
        },
        "mime": [
            "^text",
            "osjs\\/document"
        ],
        "category": "office",
        "icon": "apps/libreoffice34-writer.png",
        "compability": [
            "richtext"
        ],
        "preload": [
            {
                "type": "javascript",
                "src": "combined.js"
            },
            {
                "type": "stylesheet",
                "src": "combined.css"
            },
            {
                "src": "scheme.html",
                "type": "scheme"
            }
        ],
        "type": "application",
        "path": "default/Writer",
        "build": {},
        "repo": "default"
    }
});
  };
})();
