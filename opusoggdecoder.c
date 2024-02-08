#include <stdio.h>
#include <string.h>
#include "ogg_util.h"




uint8_t ogg_stream_read(ogg_stream_t stream, uint8_t * data, uint8_t data_len)
{
  return fread(data, 1, data_len,  (FILE *)stream);
}

uint8_t ogg_stream_write(ogg_stream_t stream, uint8_t * data, uint16_t data_len)
{
  return fwrite(data, 1, data_len, (FILE *)stream);
}


int main(int argc, char* argv[])
{
  FILE * rfile;
  ogg_decoder_t ogg_decoder;
  uint8_t segment[65536];
  uint8_t segment_size;
  opus_head_t * opus_head;
  opus_tags_t * opus_tags;
  char txt[255];
  uint32_t i, user_comment_list_length;
  uint8_t * ptr;
  int rc;

#ifdef _DEBUG
  argc = 2;
  argv[1] = "opusoggdecoder/demo.opus";
#endif

  if (argc < 2) {
    printf("use: opusoggdecoder opusfile\r\n");
    return 0;
  }

  rfile = fopen(argv[1], "rb");
  if (!rfile) {
    printf("can`t open opusfile\r\n");
    return 0;
  }


  ogg_decoder_init(&ogg_decoder, (ogg_stream_t)rfile, ogg_stream_read);

  while (ogg_next_segment(&ogg_decoder, segment, &segment_size) != OGG_RESULT_ERROR_READ_STREAM_EOFF) {


     if ( (ogg_decoder.page.header_type == OGG_PAGE_HEADER_TYPE_BEGIN_OF_STREAM) &&
          (ogg_decoder.page.number == 0) &&
          (memcmp(segment, OGG_OPUS_HEAD_SIGNATURE, strlen(OGG_OPUS_HEAD_SIGNATURE)) == 0) ) {

         opus_head = (opus_head_t *)segment;

         printf("Opus head read, frequency:% lu, channels: %d\r\n", opus_head->sample_rate, opus_head->channels_count);
         continue;

    }
    else if (memcmp(segment, OGG_OPUS_TAGS_SIGNATURE, strlen(OGG_OPUS_TAGS_SIGNATURE)) == 0) {

         opus_tags = (opus_tags_t *)segment;

         ptr = get_opus_tag(opus_tags->data, txt, sizeof(txt));
         printf("OpusTag Vendor: %s\r\n", txt);

         user_comment_list_length = *(uint32_t *)ptr;
         ptr += sizeof(uint32_t);
         for (i = 0; i < user_comment_list_length; i++) {
           ptr = get_opus_tag(ptr, txt, sizeof(txt));
           printf("OpusTags User: %s\r\n", txt);
         }
         continue;
    }
    else {

       rc = segment_size;

       while (segment_size == 255) {
            if (ogg_next_segment(&ogg_decoder, segment + rc, &segment_size) == OGG_RESULT_ERROR_READ_STREAM_EOFF) break;
            rc += segment_size;
       }


        printf("Opus data %d bytes for decode\r\n", rc);

    }

  } //end while




   fclose(rfile);

   return 0;
}


