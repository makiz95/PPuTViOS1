#include "graphics_controller.h"
#include <directfb.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include "pthread.h"


static IDirectFBSurface* primary = NULL;
static IDirectFB* dfbInterface = NULL;
static DFBSurfaceDescription surfaceDesc;
static DFBFontDescription fontDesc;
static IDirectFBFont* fontInterface = NULL;
static int32_t screenWidth = 0;
static int32_t screenHeight = 0;
static uint8_t stopDrawing = 0;

static pthread_t gcThread;
static pthread_mutex_t graphicsMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t graphicsCond = PTHREAD_COND_INITIALIZER;
static DrawComponents componentsToDraw;

static timer_t programNumberTimer;
static struct itimerspec programTimerSpec;
static struct itimerspec programTimerSpecOld;
static struct sigevent programSignalEvent;

static timer_t volumeTimer;
static struct itimerspec volumeTimerSpec;
static struct itimerspec volumeTimerSpecOld;
static struct sigevent volumeSignalEvent;

static timer_t infoTimer;
static struct itimerspec infoTimerSpec;
static struct itimerspec infoTimerSpecOld;
static struct sigevent infoSignalEvent;

static int32_t timerFlags = 0;

static void removeProgramNumber();
static void removeVolumeBar();
static void removeInfo();
static void* renderThread();
static void setTimerParams();
static void wipeScreen();
static uint16_t YearToDraw = 1000;
static uint8_t tmpMonthToDraw = 0;
static uint8_t dayToDraw = 0;
//static uint8_t hoursToDraw = 0;
//static uint8_t minutesToDraw = 0;
static int16_t audioPidToDraw = 0;
static int16_t videoPidToDraw = 0;
static IDirectFBImageProvider *provider;
static IDirectFBSurface *logoSurface = NULL;
static int32_t logoHeight;
static int32_t logoWidth;

/* helper macro for error checking */
#define DFBCHECK(x...)                                      \
{                                                           \
DFBResult err = x;                                          \
                                                            \
if (err != DFB_OK)                                          \
  {                                                         \
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );  \
    DirectFBErrorFatal( #x, err );                          \
  }                                                         \
}


GraphicsControllerError graphicsControllerInit()
{
	/* initialize DirectFB */
	if (DirectFBInit(0, NULL))
	{
		return GC_ERROR;
	}

	setTimerParams();

	componentsToDraw.showProgramNumber = false;
	componentsToDraw.showVolume = false;
	componentsToDraw.showInfo = false;
	componentsToDraw.showChannelDial = false;
	componentsToDraw.programNumber = 0;
	componentsToDraw.volume = 0;

	/* fetch the DirectFB interface */
	if (DirectFBCreate(&dfbInterface))
	{
		return GC_ERROR;
	}

	/* tell the DirectFB to take the full screen for this application */
	if (dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN))
	{
		return GC_ERROR;
	}

    /* create primary surface with double buffering enabled */
	surfaceDesc.flags = DSDESC_CAPS;
	surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
	if (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary))
	{
		return GC_ERROR;
	}    
    
    /* fetch the screen size */
    if (primary->GetSize(primary, &screenWidth, &screenHeight))
	{
		return GC_ERROR;
	}

	fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = 50;

	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));


	if (pthread_create(&gcThread, NULL, &renderThread, NULL))
    {
        printf("Error creating input event task!\n");
        return GC_THREAD_ERROR;
    }

	return GC_NO_ERROR;
}

GraphicsControllerError graphicsControllerDeinit()
{
	stopDrawing = 1;

	if (ETIMEDOUT == pthread_cond_wait(&graphicsCond, &graphicsMutex))
	{
		printf("Error neki koji god\n");
	}

	if (pthread_join(gcThread, NULL))
    {
        printf("\n%s : ERROR pthread_join fail!\n", __FUNCTION__);
        return GC_THREAD_ERROR;
    }

	printf("GCTHREAD joined!\n");

	timer_delete(programNumberTimer);
	timer_delete(volumeTimer);
	timer_delete(infoTimer);

	primary->Release(primary);
	dfbInterface->Release(dfbInterface);
	return GC_NO_ERROR;

}

void* renderThread()
{
	char tempString[10];

	while (!stopDrawing)
	{
		wipeScreen();

		if (componentsToDraw.showVolume)
		{
    		/* create the image provider for the specified file */
			switch (componentsToDraw.volume)
			{
				case 0:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_0.png", &provider));
					break;
				case 1:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_1.png", &provider));
					break;
				case 2:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_2.png", &provider));
					break;
				case 3:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_3.png", &provider));
					break;
				case 4:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_4.png", &provider));
					break;
				case 5:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_5.png", &provider));
					break;
				case 6:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_6.png", &provider));
					break;
				case 7:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_7.png", &provider));
					break;
				case 8:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_8.png", &provider));
					break;
				case 9:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_9.png", &provider));
					break;
				case 10:
					DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_10.png", &provider));
					break;
			}

    		/* get surface descriptor for the surface where the image will be rendered */
			DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
    		/* create the surface for the image */
			DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &logoSurface));
    		/* render the image to the surface */
			DFBCHECK(provider->RenderTo(provider, logoSurface, NULL));
	
    		/* cleanup the provider after rendering the image to the surface */
			provider->Release(provider);
	
    		/* fetch the logo size and add (blit) it to the screen */
			DFBCHECK(logoSurface->GetSize(logoSurface, &logoWidth, &logoHeight));
		DFBCHECK(primary->Blit(primary, logoSurface, NULL, screenWidth - logoWidth - 100, 350));
		}

		if (componentsToDraw.showInfo)
		{
			primary->SetColor(primary, 0xe0, 0x91, 0xd7, 0xEF);
    		primary->FillRectangle(primary, 3*screenWidth/10, 3*screenHeight/4, 4*screenWidth/10, screenHeight/5);

			DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xFF));

			sprintf(tempString, "Video PID : %d", videoPidToDraw);

			DFBCHECK(primary->DrawString(primary, tempString, -1, 3*screenWidth/9 - 50, 3*screenHeight/4 + 40, DSTF_LEFT));

			sprintf(tempString, "Audio PID : %d", audioPidToDraw);

			DFBCHECK(primary->DrawString(primary, tempString, -1, 3*screenWidth/9 - 50, 3*screenHeight/4 + 80, DSTF_LEFT));

			if (YearToDraw == 0)
			{
				sprintf(tempString, "Date not available");
			}
			else
			{
				switch(tmpMonthToDraw)
			   	{ 
			 	case 1:
			    		sprintf(tempString, "January/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	case 2:
			    		sprintf(tempString, "February/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	case 3:
			    		sprintf(tempString, "March/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	case 4:
			    		sprintf(tempString, "April/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	case 5:
			    		sprintf(tempString, "May/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	case 6:
			    		sprintf(tempString, "June/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 7:
			    		sprintf(tempString, "July/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 8:
			    		sprintf(tempString, "August/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 9:
			    		sprintf(tempString, "September/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 10:
			    		sprintf(tempString, "October/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 11:
			    		sprintf(tempString, "November/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
				case 12:
			    		sprintf(tempString, "December/%.2d/%.4d", dayToDraw, YearToDraw);
			 		  	break;
			 	}
			}

			DFBCHECK(primary->DrawString(primary, tempString, -1, 3*screenWidth/9 - 50, screenHeight - 60, DSTF_LEFT));
		}

		if (componentsToDraw.showChannelDial)
		{
		}
		DFBCHECK(primary->Flip(primary, NULL, 0));
	}

	pthread_mutex_lock(&graphicsMutex);
	pthread_cond_signal(&graphicsCond);
	pthread_mutex_unlock(&graphicsMutex);
}

void wipeScreen()
{
    /* clear screen */
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
}

void drawProgramNumber()
{

	timer_settime(programNumberTimer, timerFlags, &programTimerSpec, &programTimerSpecOld);
	componentsToDraw.showProgramNumber = true;
}

void drawVolumeBar(uint8_t volumeValue)
{
	componentsToDraw.volume = volumeValue;
	timer_settime(volumeTimer, timerFlags, &volumeTimerSpec, &volumeTimerSpecOld);
	componentsToDraw.showVolume = true;
}

void drawInfoRect(uint8_t tmpMonth, uint8_t day, uint16_t Year, int16_t audioPid, int16_t videoPid)
{
	timer_settime(infoTimer, timerFlags, &infoTimerSpec, &infoTimerSpecOld);

	audioPidToDraw = audioPid;
	videoPidToDraw = videoPid;

	YearToDraw = Year;
	tmpMonthToDraw = tmpMonth;
	dayToDraw = day;
	/*hoursToDraw = hours;
	minutesToDraw = minutes;
	*/
	componentsToDraw.showInfo = true;
}

void setTimerParams()
{
	/* Setting timer for program number */
	programSignalEvent.sigev_notify = SIGEV_THREAD;
	programSignalEvent.sigev_notify_function = removeProgramNumber;
	programSignalEvent.sigev_value.sival_ptr = NULL;
	programSignalEvent.sigev_notify_attributes = NULL;
	timer_create(CLOCK_REALTIME, &programSignalEvent, &programNumberTimer);

	/*  */
	memset(&programTimerSpec, 0, sizeof(programTimerSpec));
	programTimerSpec.it_value.tv_sec = 4;
	programTimerSpec.it_value.tv_nsec = 0;

	/* Setting timer for volume bar */
	volumeSignalEvent.sigev_notify = SIGEV_THREAD;
	volumeSignalEvent.sigev_notify_function = removeVolumeBar;
	volumeSignalEvent.sigev_value.sival_ptr = NULL;
	volumeSignalEvent.sigev_notify_attributes = NULL;
	timer_create(CLOCK_REALTIME, &volumeSignalEvent, &volumeTimer);

    /* */
	memset(&volumeTimerSpec, 0, sizeof(volumeTimerSpec));
	volumeTimerSpec.it_value.tv_sec = 3;
	volumeTimerSpec.it_value.tv_nsec = 0;

	/* Setting timer for info bar */
	infoSignalEvent.sigev_notify = SIGEV_THREAD;
	infoSignalEvent.sigev_notify_function = removeInfo;
	infoSignalEvent.sigev_value.sival_ptr = NULL;
	infoSignalEvent.sigev_notify_attributes = NULL;
	timer_create(CLOCK_REALTIME, &infoSignalEvent, &infoTimer);

	/* */
	memset(&infoTimerSpec, 0, sizeof(infoTimerSpec));
	infoTimerSpec.it_value.tv_sec = 3;
	infoTimerSpec.it_value.tv_nsec = 0;
}

void removeProgramNumber()
{
	componentsToDraw.showProgramNumber = false;
}

void removeVolumeBar()
{
	componentsToDraw.showVolume = false;
}

void removeInfo()
{
	componentsToDraw.showInfo = false;
}

void removeChannelDial()
{
	componentsToDraw.showChannelDial = false;
}
