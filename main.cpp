#include "CastPeer.h"
#include "uv.h"

#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "SDL2/SDL.h"
}

extern uint8_t  *GlobalFrame;
SDL_Window *screen;
SDL_Renderer* sdlRenderer;
SDL_Texture* sdlTexture;
SDL_Rect sdlRect;
uint32_t GWIDTH = 0;
uint32_t GHEIGHT = 0;

int sdlInit()
{
	if(SDL_Init(SDL_INIT_VIDEO)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1920, 1080,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  

	Uint32 pixformat=0;

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat= SDL_PIXELFORMAT_IYUV;  

  return 0;
}

int sdl_render() {
  SDL_UpdateTexture( sdlTexture, NULL, GlobalFrame, 360);  

  //FIX: If window is resize
  sdlRect.x = 200;  
  sdlRect.y = 200;  
  sdlRect.w = GWIDTH;  
  sdlRect.h = GHEIGHT;  
  
  SDL_RenderClear( sdlRenderer );   
  SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
  SDL_RenderPresent( sdlRenderer );
  return 0;
}


void VideoCallback(const Cast::VideoPacket *videoPacket) {
  if (GWIDTH != videoPacket->width) {
    GWIDTH = videoPacket->width;
    GHEIGHT = videoPacket->height;
    sdlTexture = SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,GWIDTH,GHEIGHT);
  }
  sdl_render();
}

void AudioCallback(const Cast::AudioPacket *audioPacket) {
  Log("Audio: sample: %u, len: %u, timestamp: %llu\n", audioPacket->sample, audioPacket->len, audioPacket->timestamp);
}

bool callRequest(const char *peerId, const json *config, json *responseConfig) {
  (*responseConfig) = {
      {"audioBitWidth", 16},
      {"audioChannel", 1},
      {"audioCodec", "AAC"},
      {"audioRate", 16},
      {"audioSampling", 16000},
      {"videoResolution", "360 x 720"},
      {"videoCodec", "H264"},
      {"videoFps", 30},
      {"videoRate", 4000},
      {"videoGOP", 30},
  };
  return true;
}

void endCallback(const char *peerId) {
  printf("peer: %s end call\n", peerId);
}







int main() {
  sdlInit();
  av_register_all();
  avcodec_register_all();
  Cast::CastPeer *castPeer = new Cast::CastPeer("sdl", callRequest, endCallback, AudioCallback, VideoCallback);
  castPeer->Start();

  std::cin.get(); // make program not to return
}
