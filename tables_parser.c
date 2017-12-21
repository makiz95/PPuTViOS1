#include "tables.h"

ParseErrorCode parsePatHeader(const uint8_t* patHeaderBuffer, PatHeader* patHeader)
{    
    if(patHeaderBuffer==NULL || patHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    patHeader->tableId = (uint8_t)* patHeaderBuffer; 
    if (patHeader->tableId != 0x00)
    {
        printf("\n%s : ERROR it is not a PAT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    lower8Bits = (uint8_t)(*(patHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    patHeader->sectionSyntaxIndicator = lower8Bits & 0x01;

    higher8Bits = (uint8_t) (*(patHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->sectionLength = all16Bits & 0x0FFF;
    
    higher8Bits = (uint8_t) (*(patHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->transportStreamId = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    patHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    patHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 6));
    patHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 7));
    patHeader->lastSectionNumber = lower8Bits & 0xFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatServiceInfo(const uint8_t* patServiceInfoBuffer, PatServiceInfo* patServiceInfo)
{
    if(patServiceInfoBuffer==NULL || patServiceInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 1));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->programNumber = all16Bits & 0xFFFF; 

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer + 2));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 3));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->pid = all16Bits & 0x1FFF;
    
    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatTable(const uint8_t* patSectionBuffer, PatTable* patTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(patSectionBuffer==NULL || patTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePatHeader(patSectionBuffer,&(patTable->patHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PAT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 /*PAT header size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(patSectionBuffer + 8); /* Position after last_section_number */
    patTable->serviceInfoCount = 0; /* Number of services info presented in PAT table */
    
    while(parsedLength < patTable->patHeader.sectionLength)
    {
        if(patTable->serviceInfoCount > TABLES_MAX_NUMBER_OF_PIDS_IN_PAT - 1)
        {
            printf("\n%s : ERROR there is not enough space in PAT structure for Service info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePatServiceInfo(currentBufferPosition, &(patTable->patServiceInfoArray[patTable->serviceInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 4; /* Size from program_number to pid */
            parsedLength += 4; /* Size from program_number to pid */
            patTable->serviceInfoCount ++;
        }    
    }
    
    return TABLES_PARSE_OK;
}

ParseErrorCode printPatTable(PatTable* patTable)
{
    uint8_t i=0;
    
    if(patTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PAT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",patTable->patHeader.tableId);
    printf("section_length           |      %d\n",patTable->patHeader.sectionLength);
    printf("transport_stream_id      |      %d\n",patTable->patHeader.transportStreamId);
    printf("section_number           |      %d\n",patTable->patHeader.sectionNumber);
    printf("last_section_number      |      %d\n",patTable->patHeader.lastSectionNumber);
    
    for (i=0; i<patTable->serviceInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("program_number           |      %d\n",patTable->patServiceInfoArray[i].programNumber);
        printf("pid                      |      %d\n",patTable->patServiceInfoArray[i].pid); 
    }
    printf("\n********************PAT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}


ParseErrorCode parsePmtHeader(const uint8_t* pmtHeaderBuffer, PmtTableHeader* pmtHeader)
{

    if(pmtHeaderBuffer==NULL || pmtHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    pmtHeader->tableId = (uint8_t)* pmtHeaderBuffer; 
    if (pmtHeader->tableId != 0x02)
    {
        printf("\n%s : ERROR it is not a PMT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    pmtHeader->sectionSyntaxIndicator = lower8Bits & 0x01;
    
    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->sectionLength = all16Bits & 0x0FFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programNumber = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    pmtHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    pmtHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 6));
    pmtHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 7));
    pmtHeader->lastSectionNumber = lower8Bits & 0xFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 8));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 9));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->pcrPid = all16Bits & 0xFFFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 10));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 11));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programInfoLength = all16Bits & 0x0FFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtElementaryInfo(const uint8_t* pmtElementaryInfoBuffer, PmtElementaryInfo* pmtElementaryInfo)
{
    if(pmtElementaryInfoBuffer==NULL || pmtElementaryInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    pmtElementaryInfo->streamType = (uint8_t) (*(pmtElementaryInfoBuffer));

    higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 1));
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->elementaryPid = all16Bits & 0x1FFF; 

    higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 3));
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->esInfoLength = all16Bits & 0x0FFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtTable(const uint8_t* pmtSectionBuffer, PmtTable* pmtTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(pmtSectionBuffer==NULL || pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePmtHeader(pmtSectionBuffer,&(pmtTable->pmtHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PMT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 + pmtTable->pmtHeader.programInfoLength /*PMT header size*/ + 4 /*CRC size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(pmtSectionBuffer + 12 + pmtTable->pmtHeader.programInfoLength); /* Position after last descriptor */
    pmtTable->elementaryInfoCount = 0; /* Number of elementary info presented in PMT table */
    
    while(parsedLength < pmtTable->pmtHeader.sectionLength)
    {
        if(pmtTable->elementaryInfoCount > TABLES_MAX_NUMBER_OF_ELEMENTARY_PID - 1)
        {
            printf("\n%s : ERROR there is not enough space in PMT structure for elementary info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePmtElementaryInfo(currentBufferPosition, &(pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elemntary info descriptor*/
            parsedLength += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elementary info descriptor */
            pmtTable->elementaryInfoCount++;
        }    
    }

    return TABLES_PARSE_OK;
}

ParseErrorCode printPmtTable(PmtTable* pmtTable)
{
    uint8_t i=0;
    
    if(pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PMT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",pmtTable->pmtHeader.tableId);
    printf("section_length           |      %d\n",pmtTable->pmtHeader.sectionLength);
    printf("program_number           |      %d\n",pmtTable->pmtHeader.programNumber);
    printf("section_number           |      %d\n",pmtTable->pmtHeader.sectionNumber);
    printf("last_section_number      |      %d\n",pmtTable->pmtHeader.lastSectionNumber);
    printf("program_info_legth       |      %d\n",pmtTable->pmtHeader.programInfoLength);
    
    for (i=0; i<pmtTable->elementaryInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("stream_type              |      %d\n",pmtTable->pmtElementaryInfoArray[i].streamType);
        printf("elementary_pid           |      %d\n",pmtTable->pmtElementaryInfoArray[i].elementaryPid);
    }
    printf("\n********************PMT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}

ParseErrorCode parseTdtTable(const uint8_t* tdtSectionBuffer, TdtTable* tdtTable)
{
	uint8_t lower8Bits = 0;
	uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    if (tdtSectionBuffer == NULL || tdtTable == NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

	tdtTable->tableId = (uint8_t)* tdtSectionBuffer;

	higher8Bits = (uint8_t) *(tdtSectionBuffer + 1);
	lower8Bits = (uint8_t) *(tdtSectionBuffer + 2);
	all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
	tdtTable->sectionLength = all16Bits & 0x0FFF;

	higher8Bits = (uint8_t) *(tdtSectionBuffer + 3);
	lower8Bits = (uint8_t) *(tdtSectionBuffer + 4);
	all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
	tdtTable->MJD = all16Bits;

	lower8Bits = (uint8_t) *(tdtSectionBuffer + 5);
	tdtTable->hours = 16*(lower8Bits >> 4) + (lower8Bits & 0x0F);

	lower8Bits = (uint8_t) *(tdtSectionBuffer + 6);
	tdtTable->minutes = 16*(lower8Bits >> 4) + (lower8Bits & 0x0F);

	lower8Bits = (uint8_t) *(tdtSectionBuffer + 7);
	tdtTable->seconds = 16*(lower8Bits >> 4) + (lower8Bits & 0x0F);

	return TABLES_PARSE_OK;
}

ParseErrorCode printTdtTable(TdtTable* tdtTable)
{
	if (tdtTable == NULL)
	{
		printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
		return TABLES_PARSE_ERROR;
	}

    printf("\n********************TDT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",tdtTable->tableId);
    printf("section_length           |      %d\n",tdtTable->sectionLength);
	printf("current time (UTC time)  |      %.2d:%.2d:%.2d\n", tdtTable->hours, tdtTable->minutes, tdtTable->seconds);
	printf("MJD code                 |      %d\n", tdtTable->MJD);
    printf("\n********************TDT TABLE SECTION********************\n");

	return TABLES_PARSE_OK;
}

ParseErrorCode parseTotTable(const uint8_t* totSectionBuffer, TotTable* totTable)
{
	uint8_t lower8Bits = 0;
	uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
	uint16_t descriptorLoopLength = 0;
	uint8_t infoLoopLength = 0;
	totTable->descriptorsCount = 0;

    if (totSectionBuffer == NULL || totTable == NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

	totTable->tableId = (uint8_t)* totSectionBuffer;

	higher8Bits = (uint8_t) *(totSectionBuffer + 1);
	lower8Bits = (uint8_t) *(totSectionBuffer + 2);
	all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
	totTable->sectionLength = all16Bits & 0x0FFF;

	higher8Bits = (uint8_t) *(totSectionBuffer + 3);
	lower8Bits = (uint8_t) *(totSectionBuffer + 4);
	all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
	totTable->MJD = all16Bits;

	totTable->hours = (uint8_t) *(totSectionBuffer + 5);
	totTable->minutes = (uint8_t) *(totSectionBuffer + 6);
	totTable->seconds = (uint8_t) *(totSectionBuffer + 7);

	higher8Bits = (uint8_t) *(totSectionBuffer + 8);
	lower8Bits = (uint8_t) *(totSectionBuffer + 9);
	all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
	totTable->descriptorsLoopLength = all16Bits & 0x0FFF;
	descriptorLoopLength = all16Bits & 0x0FFF;

	while (descriptorLoopLength > 4)
	{
		uint8_t i = 0;
		uint8_t currentDesc = totTable->descriptorsCount;
		uint8_t currentTag = (uint8_t) *(totSectionBuffer + 10 + 15*currentDesc);

		if (currentTag == 0x58)
		{
			totTable->descriptors[currentDesc].descriptorTag = currentTag;
			totTable->descriptors[currentDesc].descriptorLength = (uint8_t) *(totSectionBuffer + 11 + 15*currentDesc);
			totTable->descriptors[currentDesc].numberOfInfos = (uint8_t) (*(totSectionBuffer + 11 + 15*currentDesc)/13);

			for (i = 0; i < totTable->descriptors[currentDesc].numberOfInfos; i++)
			{
				totTable->descriptors[currentDesc].ltoInfo[i].countryCH1 = (uint8_t) *(totSectionBuffer + 12 + i*13);
				totTable->descriptors[currentDesc].ltoInfo[i].countryCH2 = (uint8_t) *(totSectionBuffer + 13 + i*13);
				totTable->descriptors[currentDesc].ltoInfo[i].countryCH3 = (uint8_t) *(totSectionBuffer + 14 + i*13);

				lower8Bits = (uint8_t) *(totSectionBuffer + 15 + i*13);
				totTable->descriptors[currentDesc].ltoInfo[i].localTimeOffsetPolarity = lower8Bits & 0x01;
				totTable->descriptors[currentDesc].ltoInfo[i].countryRegionId = lower8Bits >> 2;
			
				higher8Bits = (uint8_t) *(totSectionBuffer + 16 + i*13);
				lower8Bits = (uint8_t) *(totSectionBuffer + 17 + i*13);
				totTable->descriptors[currentDesc].ltoInfo[i].localTimeOffsetHours = (higher8Bits & 0x0F) + 10*((higher8Bits & 0xF0) >> 4);
				totTable->descriptors[currentDesc].ltoInfo[i].localTimeOffsetMinutes = (lower8Bits & 0x0F) + 10*((lower8Bits & 0xF0) >> 4);
			}
		}
		totTable->descriptorsCount++;
		descriptorLoopLength -= (totTable->descriptors[currentDesc].descriptorLength + 2);
	}

	return TABLES_PARSE_OK;	
}

ParseErrorCode printTotTable(TotTable* totTable)
{
	uint8_t i = 0;
	uint8_t j = 0;

	if (totTable == NULL)
	{
		printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
		return TABLES_PARSE_ERROR;
	}

    printf("\n********************TOT TABLE SECTION********************\n");
    printf("table_id                 |      %.2x\n",totTable->tableId);
    printf("section_length           |      %d\n",totTable->sectionLength);
	printf("current time (UTC time)  |      %.2x:%.2x:%.2x\n", totTable->hours, totTable->minutes, totTable->seconds);
	printf("MJD code                 |      %d\n", totTable->MJD);

	for (i = 0; i < totTable->descriptorsCount; i++)
	{
		printf("\n%d. descriptor:\n", i+1);
		printf("descriptor tag           |        %.2x\n", totTable->descriptors[i].descriptorTag);
		printf("descriptor length        |        %d\n", totTable->descriptors[i].descriptorLength);

		for (j = 0; j < totTable->descriptors[i].numberOfInfos; j++)
		{
			printf("###########################################\n");
			printf("%d. LTO info                              \n", j+1);
			printf("counrty code              |        %.1x%.1x%.1x\n", totTable->descriptors[i].ltoInfo[j].countryCH1, totTable->descriptors[i].ltoInfo[j].countryCH2, totTable->descriptors[i].ltoInfo[j].countryCH3);
			printf("counrty region id         |        %d\n", totTable->descriptors[i].ltoInfo[j].countryRegionId);
			printf("localTimeOffsetPolarity   |        %d\n", totTable->descriptors[i].ltoInfo[j].localTimeOffsetPolarity);
			printf("local time offset         |        %.2d:%.2d\n", totTable->descriptors[i].ltoInfo[j].localTimeOffsetHours, totTable->descriptors[i].ltoInfo[j].localTimeOffsetMinutes);
			printf("###########################################\n");
		}
	}

    printf("\n********************TOT TABLE SECTION********************\n");

	return TABLES_PARSE_OK;
}
