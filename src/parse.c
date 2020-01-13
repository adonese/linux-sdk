/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include "cJSON.h"

#define E_RESOLVE_PACK 0x0F
const char *ResponseParameters[] = {
    "status_code",
    "responseMessage",
    "referenceNumber",
    "tranFee"
};

int main()
{
    printf("Hello World\n");
    int i, res_status;
    // const char *response = "{\"res_body\":{\"terminalId\":\"18000377\",\"systemTraceAuditNumber\":34,\"clientId\":\"ACTS\",\"EBSServiceName\":\"\",\"workingKey\":\"f12a64bb1c98faa4\",\"responseMessage\":\"Approval\",\"responseStatus\":\"Successful\",\"status_code\":0,\"tranDateTime\":\"191011153010\"}}";
    const cJSON *res_body = NULL;
    cJSON *body = NULL;
    cJSON *para = NULL;
    const char *response = "HTTP/1.1 200 OK\nServer: nginx/1.14.0 (Ubuntu)\nDate: Wed, 16 Oct 2019 08:20:23 GMT\nContent-Type: application/json; charset=utf-8\nContent-Length: 247\nConnection: keep-alive\nAccess-Control-Allow-Origin: *{}";
    char *status_code = NULL;
    
    
    status_code = strstr(response, "HTTP/1.1 ");
    if(status_code == NULL)
        goto end;
    
    if(memcmp(status_code+9, "200", 3) != 0){
        printf("%s\n", "Failed Tran");
        res_status = 0;
    }
    else{
        printf("%s\n", "200 OK");
        res_status = 1;
    }
    
    response = strstr(response, "{");
    if(response == NULL)
        goto end;
        
    body = cJSON_Parse(response);
    if(body == NULL){
        const char *err = cJSON_GetErrorPtr();
        if(err != NULL)
            printf("Error Before: %s", err);
        goto end;
    }
    
    if(res_status){
        res_body = cJSON_GetObjectItem(body, "ebs_response");
    
        if( (res_body == NULL) && cJSON_IsNull(res_body) )
            goto end;
                
        para = cJSON_GetObjectItem(res_body, "referenceNumber");
        if( (para != NULL) && cJSON_IsString(para) ){
            printf("refNo: %s\n", para->valuestring);
        }
        
        para = cJSON_GetObjectItem(res_body, "tranFee");
        if( (para != NULL) && cJSON_IsNumber(para) ){
            printf("tranFee: %lf\n", para->valuedouble);
        }
        
        para = cJSON_GetObjectItem(res_body, "status_code");
        if( (para != NULL) && cJSON_IsNumber(para) ){
            printf("status_code: %d\n", para->valueint);
        }
        
        para = cJSON_GetObjectItem(res_body, "responseMessage");
        if( (para != NULL) && cJSON_IsString(para) ){
            printf("responseMessage: %s\n", para->valuestring);
        }    
    }
    else{
        res_body = cJSON_GetObjectItem(body, "message");
    
        if( (res_body == NULL) || cJSON_IsNull(res_body) )
            goto end;
        
        if( memcmp(res_body->valuestring, "EBSError", 8) == 0 ){
            para = cJSON_GetObjectItem(res_body, "details");
            if( (para == NULL) || cJSON_IsNull(para) )
                goto end;
            
            para = cJSON_GetObjectItem(res_body, "responseCode");
            if( (para != NULL) && cJSON_IsNumber(para) ){
                printf("responseCode: %d\n", para->valueint);
            }
            
            para = cJSON_GetObjectItem(res_body, "responseMessage");
            if( (para != NULL) && cJSON_IsString(para) ){
                printf("responseMessage: %s\n", para->valuestring);
            }    
        }   
        
        else{
            sprintf("res_msg: %s", res_body->valuestring);
            sprintf("res_st: %s", status_code+9);
        }
                
        
    }
    
    cJSON_Delete(body);
    return 0;
    
    end:
        cJSON_Delete(body);
        sprintf("shit%s", "\n");
        return E_RESOLVE_PACK;

    // return 0;
}
