// relation_controls.h
#ifndef RELATION_CONTROLS_H
#define RELATION_CONTROLS_H

// #include "add_controls.h"



bool createRelation(const char *parentTable, const char *parentCol, const char *childTable, const char *childCol) {
    if (strlen(current_db_path) == 0) return false;

    uint8_t pTableId = getTableIndexByName(parentTable);
    uint8_t cTableId = getTableIndexByName(childTable);
    uint8_t pColId = getColumnIdByName(pTableId, parentCol);
    uint8_t cColId = getColumnIdByName(cTableId, childCol);

    if (pTableId == 0 || cTableId == 0 || pColId == 0 || cColId == 0) {
        printf("XETA: Relyasiya qurulacaq cedveller ve ya sutunlar tapilmadi!\n");
        return false;
    }

    char relPath[256];
    snprintf(relPath, sizeof(relPath), "%s/metadata/relations.db", current_db_path);
    FILE *f = fopen(relPath, "ab+");
    if (!f) return false;

    CompactRelation rel = {0, pTableId, pColId, cTableId, cColId};
    fwrite(&rel, sizeof(CompactRelation), 1, f);
    fclose(f);

    printf("Relyasiya ugurla quruldu: %s(%s) -> %s(%s)\n", parentTable, parentCol, childTable, childCol);
    return true;
}

#endif