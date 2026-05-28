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