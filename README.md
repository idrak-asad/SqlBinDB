SqlBinDB — Embedded Binary Relational Database Engine
SqlBinDB, resursları məhdud olan mikrokontrollerlər (ESP32) və çarpaz platformalı masaüstü sistemlər (Windows, Linux) üçün sıfırdan, C dilində hazırlanmış yüngül (lightweight), relyasiyalı, indeksli və yüksək qənaətli lokal binar verilənlər bazası mühərrikidir.

Heç bir xarici kitabxanadan (SQLite və s.) asılılığı yoxdur. Məlumatları birbaşa diskdə xüsusi binar formatda (.db, .idx) saxlayır, axın rejimində (stream processing) işləyir və RAM-ı yükləmədən birbaşa binar axtarış tətbiq edir.

 Əsas Üstünlüklər və Yaddaşa Qənaət Arxitekturası
Zero-RAM & Stream Processing: Böyük cədvəlləri oxuyarkən bütün datanı RAM-a yükləmir. fseek və fread funksiyaları ilə birbaşa disk üzərində sətir-sətir gəzir. RAM-da sadəcə 1 sətirlik bufer (rowSize) saxlanılır.

İtkisiz Dairəvi Model (Soft-Delete & Reuse): Silinən sətirlər diskdən fiziki olaraq dərhal silinib fraqmentasiya yaratmır. Sətrin başındakı is_deleted bayrağı 1 edilir. Yeni məlumat daxil edilərkən (INSERT), sistem cədvəli yoxlayır və ilk tapdığı silinmiş boş sətir yerini rəsmi olaraq yenidən istifadə edir (Data Reuse).

Binar Kompakt Format: Sətirlər diskdə struktur səviyyəsində (#pragma pack(push, 1)) sıxılmış halda, heç bir boşluq (padding) buraxılmadan sırf binar olaraq saxlanılır. Bu, flash yaddaşın aşınmasını minimuma endirir.

Çarpaz Platforma Dəstəyi (Cross-Platform VFS): ESP32-də daxili LittleFS virtual fayl sistemi ilə tam inteqrasiya olunub standart fopen bloklanmalarını aradan qaldırır. Eyni kod heç bir dəyişiklik edilmədən Windows və Linux-da da çalışır.

 Modullar və Funksionallıqlar
1. Database (Baza) İdarəetməsi (db_controls.h)
Mərkəzi reyestr sistemi (master_dbs.db) vasitəsilə eyni mühitdə çoxlu verilənlər bazası yaratmağa və idarə etməyə imkan verir. təhlükəsizlik üçün şifrələmə metadatası dəstəklənir.

initSystem(): LittleFS və ya lokal diski başladır, mərkəzi reyestri yoxlayır və sazlayır.

createDb(DbName, DbPsw, reCreate): Şifrəli mərkəzi qeydiyyatla yeni baza və onun daxili metadata qovluq strukturlarını yaradır.

connectDb(DbName, DbPsw): Şifrəni yoxlayaraq müvafiq bazaya təhlükəsiz qoşulur və cari aktiv yolu (current_db_path) tənzimləyir.

dropDb(DbName, DbPsw): Bazanı mərkəzi reyestrdən silir.

2. Table (Cədvəl) Sxemləri (table_controls.h)
Dinamik binar sxem idarəetməsi. Hər bir cədvəl yaradılarkən onun başlıq bloku (DBHeader) və sütun konfiqurasiyaları (ColumnConfig) faylın daxilinə binar olaraq həkk olunur.

createTable(...): Sütun adları, tipləri (INT, UINT32, UINT8, CHAR) və məhdudiyyətləri (Constraints) qəbul edərək binar cədvəl yaradır.

dropTable(tableName, hardDrop): Cədvəli silir. CASCADE funksionallığı sayəsində əgər digər cədvəllərlə əlaqəsi varsa, zəncirvari silmə tətbiq edə bilir.

selectTables(tableName): Cədvəlin sxemini (sütun adları, tip ID-ləri, ölçüləri və limitləri) terminala vizual olaraq çıxarır.

3. Sürətli İndeksləmə Modulu (index_controls.h)
Axtarış sürətini artırmaq üçün istənilən sütun üzrə manual olaraq .idx binar indeks faylları yaradıla bilər.

createIndex(tableName, columnName): Sütun üçün xüsusi binar indeks faylı təyin edir və mərkəzi indexes.db reyestrinə bağlayır.

resetTableIndexes(tableName): Cədvəl silindikdə və ya yenidən yaradıldıqda ona bağlı bütün fiziki indeks fayllarını təmizləyir.

4. Relyasiyalar və Xarici Açarlar (relation_controls.h)
Cədvəllər arasında One-to-Many (Birin-Çoxa) və ya Many-to-Many (Çoxun-Çoxa) əlaqələr qurulmasını təmin edir. Məlumatın bütövlüyünü (Referential Integrity) qoruyur.

createRelation(parentTable, parentCol, childTable, childCol): Valideyn və uşaq cədvəlləri sütun ID-ləri səviyyəsində rəsmi olaraq bir-birinə bağlayır (relations.db).

5. CRUD və Gelişmiş Məlumat Əməliyyatları (data_controls.h)
Create (Insert): insertRows() funksiyası məlumatı daxil edərkən PRIMARY KEY (Unikallıq), NOT NULL və UNIQUE məhdudiyyətlərini (Constraints) binar səviyyədə yoxlayır. Boş yer varsa oraya yazır, yoxdursa faylın sonuna əlavə edir.

Read (Select): Şərtli və şərtsiz oxuma. where filtrləri (=, >, <, !, LIKE) dəstəklənir.

Update: Mövcud sətirlərin məlumatlarını disk üzərində birbaşa modifikasiya edir.

Delete: deleteRows(..., hardDelete) funksiyası unikal parametrlərə malikdir:

hardDelete = 1 (CASCADE): Valideyn cədvəldən sətir silindikdə, uşaq cədvəldə ona bağlı olan bütün sətirləri rekursiv olaraq avtomatik silir.

hardDelete = 2 (RESTRICT/SET NULL): Uşaq cədvəllərə toxunmadan birbaşa özünü soft-delete edir.
