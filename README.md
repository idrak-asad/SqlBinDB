# SqlBinDB — Ultra-Lightweight Embedded Binary Relational Database Engine

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Linux%20%7C%20Windows-blue)](https://github.com/)
[![Language](https://img.shields.io/badge/language-C%20%2F%20C%2B%2B-green)](https://en.wikipedia.org/wiki/C_(programming_language))

SqlBinDB is a zero-dependency, high-performance embedded relational database engine written entirely in pure C. Designed from scratch for resource-constrained systems like **ESP32 microcontrollers**, it also seamlessly scales to **Linux and Windows** host environments. 

By utilizing a custom, dense binary file format (`.db`, `.idx`), SqlBinDB eliminates the overhead of heavy external SQL engines while maintaining native support for relational mapping, indexing, and transactional integrity on local storage.

---

## 🚀 Key Features & Memory Optimization Architecture

### 📉 Zero-RAM & Stream Processing
Unlike databases that load complete tables into heap memory, SqlBinDB operates via a localized cursor stream. It utilizes low-level `fseek()` and `fread()` structures to scan data row-by-row directly from the flash/disk. Memory consumption remains strictly constant ($O(1)$ space complexity), holding only a single row buffer (`rowSize`) in RAM regardless of the database size.

### 🔄 Zero-Fragmentation Circular Model (Data Reuse)
To avoid standard filesystem fragmentation and unnecessary flash wear cycles on IoT devices, SqlBinDB implements a smart **Soft-Delete** mechanism. Deleted records are flagged instantly via a 1-byte header (`is_deleted = 1`). During a new `INSERT` operation, the engine scans the table sequentially and automatically reuses the first available soft-deleted slot before appending to the end of the file.

### 🗜️ Ultra-Compact Binary Storage
Data structures are directly mapped to disk byte-for-byte using data alignment override counters (`#pragma pack(push, 1)`). This eliminates structural padding bytes, reducing file footprint up to 60% compared to JSON or plain-text CSV logs, drastically lowering flash memory footprint on microcontrollers.

### 🌐 Cross-Platform VFS Integration
Equipped with an advanced hardware abstraction layer, it maps native standard I/O streams directly to the **ESP32 Virtual File System (VFS) over LittleFS**, resolving any hidden configuration locks. The identical source code compiles natively on desktop Linux and Windows targets without single-line mutations.

---

## 🛠️ Modular Subsystem Capabilities

### 1. Database Registry Controller (`db_controls.h`)
Manages multi-tenant workspace catalogs using a secure central registry hub (`master_dbs.db`).
* `initSystem()`: Mounts virtual storage partitions (LittleFS/Lokal) and initializes system state paths.
* `createDb(DbName, DbPsw, reCreate)`: Generates structured binary tracking arrays isolated with password verification schemas.
* `connectDb(DbName, DbPsw)`: Attaches system scope variables to the targeted runtime data lane.

### 2. High-Density Table Schema Controller (`table_controls.h`)
Handles dynamic binary schemas directly embedded into the table file header segment.
* `createTable(...)`: Instantiates a binary structure defining explicit datatypes (`INT`, `UINT32`, `UINT8`, `CHAR`) alongside targeted field constraints.
* `dropTable(tableName, hardDrop)`: Completely deletes tables with built-in `CASCADE` modules to purge dependencies safely.
* `selectTables(tableName)`: Formats and displays internal raw schemas visually into standard execution logs.

### 3. Accelerated Indexing Engine (`index_controls.h`)
Speeds up data fetching pipelines using custom B-tree alternative binary fast-lookup index tracks.
* `createIndex(tableName, columnName)`: Generates automated `.idx` transactional indexing structures bound directly to the database metadata layer.
* `resetTableIndexes(tableName)`: Clears physical binary index tracks on table recreation or truncation loops.

### 4. Relational Blueprint Mapper (`relation_controls.h`)
Maintains data integrity layout layers across complex storage grids.
* `createRelation(parentTable, parentCol, childTable, childCol)`: Registers operational strict **One-to-Many** and **Many-to-Many** data mappings directly into backend engine constraints (`relations.db`).

### 5. Advanced CRUD & Constraints Framework (`data_controls.h`)
* **Create (Insert):** Performs real-time binary assertions enforcing `PRIMARY KEY` uniqueness, `NOT NULL`, and `UNIQUE` validation structures before commitment pipelines.
* **Read (Select):** Stream-searches rows filtered through optimized conditional logic blocks (`=`, `>`, `<`, `!`, `LIKE` string matching tokens).
* **Update:** Edits binary byte sequences in-place inside the raw file sectors without affecting adjacent blocks.
* **Delete:** Controls memory purging with specific cascading logic parameters:
  * `hardDelete = 1 (CASCADE)`: Recursively tracks and purges linked relational child data fields safely if a parent row is deleted.
  * `hardDelete = 2 (RESTRICT / SET NULL)`: Isolates deletion routines strictly inside the scope of the target table.

---

## 📋 Technical Blueprint Specifications

| Metric / Feature | Specification / Capability |
| :--- | :--- |
| **Language Standards** | Pure ANSI C / C++ Compatible Compilation |
| **Max Column Capacity**| Up to 15 customized user-defined fields per schema |
| **Supported Primitives**| 8-bit, 32-bit Integers (Signed/Unsigned), Fixed Character Arrays |
| **Maximum Record Size** | Bound only by available physical Flash/Disk limits |
| **Dependency Context** | 100% Standalone (No SQLite, No external library requirements) |
| **Storage Engine Type**| Direct-to-Disk Binary Serialization Scheme |





# SqlBinDB — Ultra-Yüngül Daxili Binar Relyasiyalı Verilənlər Bazası Mühərriki

[![Lisenziya: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platforma](https://img.shields.io/badge/platform-ESP32%20%7C%20Linux%20%7C%20Windows-blue)](https://github.com/)
[![Dil](https://img.shields.io/badge/language-C%20%2F%20C%2B%2B-green)](https://en.wikipedia.org/wiki/C_(programming_language))

SqlBinDB — resursları məhdud olan sistemlər (xüsusilə **ESP32 mikrokontrollerləri**) və çarpaz platformalı masaüstü mühitlər (**Linux, Windows**) üçün sıfırdan, sırf **C dilində** hazırlanmış, heç bir xarici asılılığı olmayan (zero-dependency) yüksək performanslı daxili relyasiyalı verilənlər bazası mühərrikidir.

Xüsusi olaraq hazırlanmış sıx binar fayl formatı (`.db`, `.idx`) sayəsində SqlBinDB, ağır xarici SQL mühərriklərinin yaratdığı əlavə yükü tamamilə aradan qaldırır. Bununla belə, lokal yaddaş üzərində relyasiyalı əlaqələri, indeksləməni və kaskadlı struktur bütövlüyünü tam dəstəkləyir.

---

## 🚀 Əsas Üstünlüklər və Yaddaş Optimizasiyası Arxitekturası

### 📉 Zero-RAM və Axın Rejimində Oxuma (Stream Processing)
Cədvəlləri tamamilə RAM (Heap) yaddaşına yükləyən ənənəvi verilənlər bazalarından fərqli olaraq, SqlBinDB lokal kursor axını ilə işləyir. Aşağı səviyyəli `fseek()` və `fread()` funksiyalarından istifadə edərək məlumatları birbaşa flash yaddaşdan/diskdən sətir-sətir skan edir. Verilənlər bazasının ölçüsündən asılı olmayaraq RAM sərfi sabit qalır ($O(1)$ space complexity) və operativ yaddaşda sadəcə bir sətirlik bufer (`rowSize`) saxlanılır.

### 🔄 Sıfır-Fraqmentasiya və Dairəvi Model (Məlumatın Yenidən İstifadəsi)
IoT cihazlarında fayl sisteminin fraqmentasiyaya uğramasının və Flash yaddaşın tez aşınmasının qarşısını almaq üçün SqlBinDB ağıllı **Soft-Delete (Yumşaq Silmə)** mexanizmini tətbiq edir. Silinən qeydlər diskdən fiziki olaraq dərhal təmizlənmir, sadəcə sətrin başındakı 1 baytlıq başlıq bayrağı `is_deleted = 1` edilir. Yeni məlumat əlavə edilərkən (`INSERT`), mühərrik cədvəli ardıcıl yoxlayır və faylın sonuna yeni sətir açmaqdan öncə, tapdığı ilk silinmiş boş yuvanı rəsmi olaraq yenidən istifadə edir.

### 🗜️ Ultra-Sıx binar Saxlama Formatı
Məlumat strukturları yaddaşda verilənlərin düzləndirilməsi qaydalarını ləğv edərək byte-ba-byte birbaşa diskə həkk olunur (`#pragma pack(push, 1)`). Bu, strukturlar arası boşluq baytlarını (padding) tamamilə sıfırlayır və faylın ölçüsünü JSON və ya adi mətn tipli CSV loqları ilə müqayisədə 60%-ə qədər kiçildərək mikrokontrollerlərin daxili flash yaddaşına maksimum qənaət edir.

### 🌐 Çarpaz Platformalı VFS İnteqrasiyası
Təkmilləşdirilmiş aparat abstraksiya təbəqəsi (HAL) sayəsində standart daxili I/O axınlarını birbaşa **ESP32 Virtual Fayl Sistemi (VFS) üzərindən LittleFS**-ə bağlayır və gizli konfiqurasiya kilidlərini həll edir. Eyni kod bazası heç bir dəyişiklik edilmədən masaüstü Linux və Windows mühitlərində də yerli olaraq dərhal kompilyasiya olunur.

---

## 🛠️ Modulyar Alt Sistemlər və Funksionallıqlar

### 1. Database (Baza) Reyestr Modulu (`db_controls.h`)
Mərkəzi idarəetmə mərkəzi (`master_dbs.db`) vasitəsilə eyni mühitdə çoxlu verilənlər bazası kataloqlarını və onların təhlükəsizliyini idarə edir.
* `initSystem()`: Virtual yaddaş bölmələrini (LittleFS/Lokal disk) başladır və sistem qovluq strukturlarını sazlayır.
* `createDb(DbName, DbPsw, reCreate)`: Şifrəli mərkəzi qeydiyyatla təcrid olunmuş yeni binar baza strukturlarını və daxili metadata qovluqlarını yaradır.
* `connectDb(DbName, DbPsw)`: Giriş şifrəsini doğrulayaraq sistemi cari aktiv işçi yoluna (`current_db_path`) bağlayır.

### 2. Yüksək Sıxlıqlı Table (Cədvəl) Sxemləri (`table_controls.h`)
Cədvəl sxemlərini dinamik olaraq birbaşa cədvəl faylının daxili başlıq seqmentinə binar olaraq qeyd edir.
* `createTable(...)`: Sütun tiplərini (`INT`, `UINT32`, `UINT8`, `CHAR`) və sahə məhdudiyyətlərini qəbul edərək fiziki binar cədvəl formatını strukturlaşdırır.
* `dropTable(tableName, hardDrop)`: Cədvəlləri sistemdən tamamilə silir; daxili `CASCADE` modulu sayəsində asılılıqları təhlükəsiz şəkildə təmizləyir.
* `selectTables(tableName)`: Cədvəlin daxili raw sxemini və sütun konfiqurasiyalarını vizual olaraq terminal loqlarına çıxarır.

### 3. Sürətləndirilmiş İndeksləmə Mühərriki (`index_controls.h`)
Axtarış və süzgəcləmə əməliyyatlarını sürətləndirmək üçün xüsusi binar sürətli keçid indeks xətləri yaradır.
* `createIndex(tableName, columnName)`: Müvafiq sütun üçün avtomatlaşdırılmış `.idx` binar indeks faylı formalaşdırır və mərkəzi metadata qatına bağlayır.
* `resetTableIndexes(tableName)`: Cədvəl silindikdə və ya yenidən yaradıldıqda ona aid olan bütün fiziki binar indeks fayllarını diskdən təmizləyir.

### 4. Relyasiyalı Struktur Xəritəsi (`relation_controls.h`)
Mürəkkəb saxlama şəbəkələrində məlumatların bir-biri ilə əlaqəsini və bütövlüyünü (Referential Integrity) qoruyur.
* `createRelation(parentTable, parentCol, childTable, childCol)`: Cədvəllər arasında **One-to-Many (Birin-Çoxa)** və **Many-to-Many (Çoxun-Çoxa)** əlaqə strukturlarını backend səviyyəsində birbaşa `relations.db` faylına qeyd edir.

### 5. Təkmil CRUD və Məhdudiyyətlər Çərçivəsi (`data_controls.h`)
* **Create (Insert):** Məlumat yazılmazdan öncə `PRIMARY KEY` (Unikallıq), `NOT NULL` və `UNIQUE` binar məhdudiyyətlərini real vaxt rejimində yoxlayır və təsdiqləyir.
* **Read (Select):** Sətirləri daxili şərt mühərriki (`=`, `>`, `<`, `!`, `LIKE` mətn axtarışı) vasitəsilə diskdən axın rejimində süzgəcdən keçirərək oxuyur.
* **Update:** Mövcud binar bayt ardıcıllıqlarını digər bloklara toxunmadan birbaşa fayl daxilindəki ünvanında (in-place) modifikasiya edir.
* **Delete:** Məlumatların silinməsini xüsusi relyasiyalı kaskad parametrləri ilə idarə edir:
  * `hardDelete = 1 (CASCADE)`: Valideyn sətir silindikdə, uşaq cədvəldə ona bağlı olan bütün verilənləri rekursiv olaraq avtomatik tapır və silir.
  * `hardDelete = 2 (RESTRICT / SET NULL)`: Silmə əməliyyatını digər cədvəllərə toxunmadan sırf hədəf cədvəlin öz daxilində izolyasiya edir.

---

## 📋 Texniki Parametrlər və Spesifikasiyalar

| Göstərici / Özəllik | Texniki İmkan və Tutum |
| :--- | :--- |
| **Dil Standartı** | Saf ANSI C / C++ uyğun kompilyasiya |
| **Maksimum Sütun Sayı**| Sxem üzrə 15 ədəd xüsusi təyin olunmuş sahə |
| **Dəstəklənən Tiplər**| 8-bit, 32-bit Tam Ədədlər (Signed/Unsigned), Sabit Simvol Massivləri |
| **Maksimum Sətir Tutumu** | Hədd yoxdur (Sırf fiziki Flash/Disk tutumu qədər) |
| **Asılılıq Rejimi** | 100% Standalone (Nə SQLite, nə də başqa kitabxana tələb olunmur) |
| **Saxlama Modeli**| Birbaşa Diskə Sıxlaşdırılmış Binar Serializasiya Sxemi |



