
#include "ogg_util.h"
#include <string.h>

void ogg_decoder_init(ogg_decoder_t * ogg_decoder, ogg_stream_t stream, ogg_stream_read_t * stream_read)
{
  ogg_decoder->stream = stream;
  ogg_decoder->stream_read = stream_read;
  ogg_decoder->page_load = FALSE;
  ogg_decoder->segment_index = 0;
#ifdef OGG_STATISTIC
  ogg_decoder->stat_pages_cnt = 0;
  ogg_decoder->stat_segments_cnt = 0;
  ogg_decoder->stat_bytes_cnt = 0;
#endif  
}


ogg_result_t ogg_read_page_data(ogg_decoder_t * ogg_decoder, ogg_page_t * ogg_page)
{
    uint8_t len, rc;

    rc = ogg_decoder->stream_read(ogg_decoder->stream, (uint8_t *)&ogg_page->capture_pattern, sizeof(ogg_page->capture_pattern));

    if (!rc) return OGG_RESULT_ERROR_READ_STREAM_EOFF;

    if (rc != sizeof(ogg_page->capture_pattern)) return OGG_RESULT_ERROR_READ_STREAM_CNT;

    if (ogg_page->capture_pattern != OGG_CAPTURE_PATTERN_UINT) return OGG_RESULT_ERROR_CAPTURE_PATTERN;

    len = sizeof(ogg_page_t) - sizeof(ogg_page->capture_pattern) - sizeof(ogg_page->segment_size);

    rc = ogg_decoder->stream_read(ogg_decoder->stream, (uint8_t *)&ogg_page->version, len);

    if (!rc) return OGG_RESULT_ERROR_READ_STREAM_EOFF;

    if (rc != len) return OGG_RESULT_ERROR_READ_STREAM_CNT;


    rc = ogg_decoder->stream_read(ogg_decoder->stream, (uint8_t *)ogg_page->segment_size, ogg_page->segments_cnt);

    if (!rc) return OGG_RESULT_ERROR_READ_STREAM_EOFF;

    if (rc != ogg_page->segments_cnt) return OGG_RESULT_ERROR_READ_STREAM_CNT;

    return OGG_RESULT_NO_ERROR;
}




ogg_result_t ogg_next_segment(ogg_decoder_t * ogg_decoder, uint8_t * segment, uint8_t * segment_size)
{

   if (!ogg_decoder->page_load) {
      ogg_result_t res = ogg_read_page_data(ogg_decoder, &ogg_decoder->page);
      if (res != OGG_RESULT_NO_ERROR) return res;
      ogg_decoder->page_load = TRUE;
      ogg_decoder->segment_index = 0;
#ifdef OGG_STATISTIC
      ogg_decoder->stat_pages_cnt++;
#endif
      if (ogg_decoder->page.segments_cnt == 0) {
        ogg_decoder->page_load = FALSE;
        ogg_decoder->segment_index = 0;
        return ogg_next_segment(ogg_decoder, segment, segment_size);
      }
   }


   if (ogg_decoder->page.segment_size[ogg_decoder->segment_index] == 0) {
        *segment_size = 0;
        ogg_decoder->segment_index++;
#ifdef OGG_STATISTIC
        ogg_decoder->stat_segments_cnt++;
#endif
        return OGG_RESULT_NO_ERROR;
   }

   *segment_size = ogg_decoder->stream_read(ogg_decoder->stream, segment, ogg_decoder->page.segment_size[ogg_decoder->segment_index]);
    if (!(*segment_size)) return OGG_RESULT_ERROR_READ_STREAM_EOFF;
   if (*segment_size != ogg_decoder->page.segment_size[ogg_decoder->segment_index]) return OGG_RESULT_ERROR_READ_STREAM_CNT;
   ogg_decoder->segment_index++;
#ifdef OGG_STATISTIC
   ogg_decoder->stat_segments_cnt++;
   ogg_decoder->stat_bytes_cnt += *segment_size;
#endif

   if (ogg_decoder->segment_index >= ogg_decoder->page.segments_cnt) {
      ogg_decoder->page_load = FALSE;
      ogg_decoder->segment_index = 0;
   }

   return OGG_RESULT_NO_ERROR;
}


ogg_result_t ogg_write_page(ogg_stream_t stream, ogg_stream_write_t * ogg_stream_write, ogg_page_t * ogg_page, uint8_t segments_data[256][256])
{

   uint16_t i, len;

   ogg_page->checksum = 0;

   len = sizeof(ogg_page_t) - sizeof(ogg_page->segment_size) + ogg_page->segments_cnt;

   ogg_stream_write(stream, (uint8_t *)ogg_page, len);

   for (i = 0; i < ogg_page->segments_cnt; i++) {
     ogg_stream_write(stream, segments_data[i], ogg_page->segment_size[i]);
   }

   return OGG_RESULT_NO_ERROR;
}



//------------------------- OPUS -----------------------------------------------

uint8_t * get_opus_tag(uint8_t * data, char * buf, uint8_t buf_size)
{
   uint32_t * plen, len;

   plen = (uint32_t *)data;

   len = MIN(*plen, (uint32_t)(buf_size - 1));

   memcpy(buf, data + sizeof(uint32_t), len);
   buf[len] = '\0';

   return  data + sizeof(uint32_t) + *plen;
}

uint8_t * add_opus_tag(uint8_t * opus_data, uint8_t opus_data_size, char * tag)
{
   uint32_t * plen;

   plen = (uint32_t *)opus_data;
   *plen = MIN(strlen(tag), (uint32_t)(opus_data_size - 1));


   memcpy(opus_data + sizeof(uint32_t), tag, *plen);

   return opus_data + sizeof(uint32_t) + *plen;
}


