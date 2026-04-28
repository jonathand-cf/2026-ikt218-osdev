extern "C" int kernel_main(void);

#include "song/song.h"
#include "libc/stdio.h"

extern "C" void play_music() {
    Song songs[] = {
        {music_1, MUSIC_1_LEN},
    };
    size_t n_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player();

    for(size_t i = 0; i < n_songs; i++) {
        player->play_song(&songs[i]);
    }
}

extern "C" int kernel_main(){
    
    //Allocation test
    int* alloc = new int(12);
    delete alloc;
    play_music();
    
    while(1) {} 
    return 0;
}