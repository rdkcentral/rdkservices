#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    WDMP_STRING = 0,
    WDMP_INT,
    WDMP_UINT,
    WDMP_BOOLEAN,
    WDMP_DATETIME,
    WDMP_BASE64,
    WDMP_LONG,
    WDMP_ULONG,
    WDMP_FLOAT,
    WDMP_DOUBLE,
    WDMP_BYTE,
    WDMP_NONE,
    WDMP_BLOB
} DATA_TYPE;

typedef enum
{
    WDMP_TR181 = 0,
    WDMP_SNMP
} PAYLOAD_TYPE;

typedef enum
{
    WDMP_SUCCESS = 0,                    /**< Success. */
    WDMP_FAILURE,                        /**< General Failure */
    WDMP_ERR_TIMEOUT,
    WDMP_ERR_NOT_EXIST,
    WDMP_ERR_INVALID_PARAMETER_NAME,
    WDMP_ERR_INVALID_PARAMETER_TYPE,
    WDMP_ERR_INVALID_PARAMETER_VALUE,
    WDMP_ERR_NOT_WRITABLE,
    WDMP_ERR_SETATTRIBUTE_REJECTED,
    WDMP_ERR_NAMESPACE_OVERLAP,
    WDMP_ERR_UNKNOWN_COMPONENT,
    WDMP_ERR_NAMESPACE_MISMATCH,
    WDMP_ERR_UNSUPPORTED_NAMESPACE,
    WDMP_ERR_DP_COMPONENT_VERSION_MISMATCH,
    WDMP_ERR_INVALID_PARAM,
    WDMP_ERR_UNSUPPORTED_DATATYPE,
    WDMP_STATUS_RESOURCES,
    WDMP_ERR_WIFI_BUSY,
    WDMP_ERR_INVALID_ATTRIBUTES,
    WDMP_ERR_WILDCARD_NOT_SUPPORTED,
    WDMP_ERR_SET_OF_CMC_OR_CID_NOT_SUPPORTED,
    WDMP_ERR_VALUE_IS_EMPTY,
    WDMP_ERR_VALUE_IS_NULL,
    WDMP_ERR_DATATYPE_IS_NULL,
    WDMP_ERR_CMC_TEST_FAILED,
    WDMP_ERR_NEW_CID_IS_MISSING,
    WDMP_ERR_CID_TEST_FAILED,
    WDMP_ERR_SETTING_CMC_OR_CID,
    WDMP_ERR_INVALID_INPUT_PARAMETER,
    WDMP_ERR_ATTRIBUTES_IS_NULL,
    WDMP_ERR_NOTIFY_IS_NULL,
    WDMP_ERR_INVALID_WIFI_INDEX,
    WDMP_ERR_INVALID_RADIO_INDEX,
    WDMP_ERR_ATOMIC_GET_SET_FAILED,
    WDMP_ERR_METHOD_NOT_SUPPORTED,
    WDMP_ERR_SESSION_IN_PROGRESS,
    WDMP_ERR_INTERNAL_ERROR,
    WDMP_ERR_DEFAULT_VALUE
} WDMP_STATUS;

typedef struct
{
    char *name;
    char *value;
    DATA_TYPE type;
} param_t;

typedef enum
{
    GET = 0,
    GET_ATTRIBUTES,
    SET,
    SET_ATTRIBUTES,
    TEST_AND_SET,
    REPLACE_ROWS,
    ADD_ROWS,
    DELETE_ROW
} REQ_TYPE;

typedef struct
{
    char *paramNames[512];
    size_t paramCnt;
} get_req_t;

typedef struct
{	
    param_t *param;
    size_t paramCnt;
} set_req_t;

typedef struct
{	
    param_t *param;
    char *newCid;
    char *oldCid;
    char *syncCmc;
    size_t paramCnt;
} test_set_req_t;


typedef struct 
{
    size_t paramCnt;   
    char **names;
    char **values;
} TableData;

typedef struct
{
    char *objectName;
    TableData *rows;
    size_t rowCnt;
} table_req_t;

typedef struct {
    REQ_TYPE reqType;
    union {
        get_req_t *getReq;
        set_req_t *setReq;
        table_req_t *tableReq;
        test_set_req_t *testSetReq;
    } u;
} req_struct;

typedef struct  
{
    char *name;
    uint64_t start;     
    uint32_t duration;  
} money_trace_span;

typedef struct  
{
    money_trace_span *spans;
    size_t count;
} money_trace_spans;

typedef struct
{
    char **paramNames;
    size_t paramCnt;
    param_t **params;
    size_t *retParamCnt;
} get_res_t;

typedef struct
{
    char *syncCMC;
    char *syncCID;
    param_t *params;
} param_res_t;

typedef struct
{
    char *newObj;
} table_res_t;

typedef struct
{
    REQ_TYPE reqType;
    union {
        get_res_t *getRes;
        param_res_t *paramRes;	
        table_res_t *tableRes;
    } u;
    money_trace_spans *timeSpan;
    WDMP_STATUS *retStatus;
    size_t paramCnt;
} res_struct;

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  To convert json string to struct 
 * 
 *  @note If the reqObj returned is not NULL, the value pointed at by
 *      bytes must be freed using wdmp_free_req_struct() by the caller.
 *
 *  @param payload    [in]  payload JSON string to be converted 
 *  @param reqObj    [out] the resulting req_struct structure if successful
 */

void wdmp_parse_request(char * payload, req_struct **reqObj);

/**
 *  To convert json string of a given payload_type to struct
 *
 *  @note If the reqObj returned is not NULL, the value pointed at by
 *      bytes must be freed using wdmp_free_req_struct() by the caller.
 *
 *  @param payload    [in]  payload JSON string to be converted
 *  @param payload_type    [in] type of JSON payload - TR181 / SNMP /or any TBD type
 *  @param reqObj    [out] the resulting structure if successful. structure depends on the payload_type
 */
void wdmp_parse_generic_request(char * payload, PAYLOAD_TYPE payload_type, req_struct **reqObj);


/**
 *  To convert response struct to json string 

 *  @param resObj    [in]  the response structure to be converted 
 *  @param payload    [out] the resulting payload string if successful
 */
 
void wdmp_form_response(res_struct *resObj, char **payload);

/**
 *  Free the req_struct structure if allocated by the wdmp-c library.
 *
 *  @note Do not call this function on the req_struct structure if the wdmp-c
 *        library did not create the structure!
 *
 *  @param msg [in] the req_struct structure to free
 */
void wdmp_free_req_struct( req_struct *reqObj );

/**
 *  Free the res_struct structure if allocated by the wdmp-c library.
 *
 *  @note Do not call this function on the res_struct structure if the wdmp-c
 *        library did not create the structure!
 *
 *  @param msg [in] the res_struct structure to free
 */
void wdmp_free_res_struct( res_struct *resObj );

void mapWdmpStatusToStatusMessage(WDMP_STATUS status, char *result);

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */

