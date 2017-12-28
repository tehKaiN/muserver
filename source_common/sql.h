typedef struct _tSqlSelectExpr{
	char szTableName[20];
	char pFields[10][20];
	char szWhereExpr[50];
	short iLimitStart;
	short iLimitCount;
} tSqlSelectExpr;
