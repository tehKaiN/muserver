// ------ Default ports ------ //

#define DEFGAMESERVERPORT 55900 // Game Server port number
#define DEFJOINSERVERPORT 55970 // Join Server port number
#define DEFDATASERVERPORT 55960 // Game Data Server Port number
#define DEFCONNSERVERPORT 55557 // Connect Data Server Port number

// ------ String lengths ------ //

#define MAX_IDSTRING 10       // Account ID
#define MAX_GUILDNAMESTRING 8 // Guild name

// ------ DB field sizes ------ //

#define ITEMCODE_SIZE 10
#define MAX_DBINVENTORY ((8*8 + 12)*ITEMCODE_SIZE) // Inventory + Equipped
#define MAX_DBMAGIC (MAX_MAGIC*3)                  // Each skill takes 3 bytes
#define MAX_WAREHOUSEDBSIZE (120*ITEMCODE_SIZE)    // Warehouse

// ------ Skill related ------ //

#define MAX_MAGIC 20 // Max spell count

// ------ Bill related ------ //

#define CERTIFYTYPE_ACCOUNT 0 // 계정인증
#define CERTIFYTYPE_IP 1      // IP 인증

#define BILLTYPE_JUNGYANG 0		// 정량
#define BILLTYPE_JUNGACK 1	  // 정액
#define BILLTYPE_JONGYANG 2	  // 종량
#define BILLTYPE_TIME 3			  // 시간제
#define BILLTYPE_DATE 4			  // 날짜제
#define BILLTYPE_NOCHARGE 5		// 무료
