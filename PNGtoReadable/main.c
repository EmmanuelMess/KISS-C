#include <sys/stat.h>
#include "malloc.h"
#include "string.h"
#include "spng/spng.h"

int main(int argc, char * argv[]) {
  if(argc < 2) {
    return 0;
  }

  for(int i = 1; i < argc; i++) {
    FILE * file = fopen(argv[i], "rb");

    spng_ctx * ctx = spng_ctx_new(0);
    spng_set_png_file(ctx, file);

    size_t outSize;
    spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &outSize);

    struct spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);
    uint32_t imageWidth = ihdr.width;
    uint32_t imageHeight = ihdr.height;

    unsigned char * image = malloc(outSize);
    spng_decode_image(ctx, image, outSize, SPNG_FMT_RGBA8, 0);

    char * outputExtension = ".explicit4ch8b";
    char * enterFilename = strtok(argv[i], ".");
    char outFilename[strlen(enterFilename) + strlen(outputExtension)];
    sprintf(outFilename, "%s%s", enterFilename, outputExtension);
    FILE * outputFile = fopen(outFilename, "wb");

    fwrite((char*) &imageWidth, sizeof(uint32_t), 1, outputFile);
    fwrite((char*) &imageHeight, sizeof(uint32_t), 1, outputFile);
    fwrite(image, sizeof(char), outSize, outputFile);
    fclose(outputFile);

    free(image);
    spng_ctx_free(ctx);
    fclose(file);
  }
}