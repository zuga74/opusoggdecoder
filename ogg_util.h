#ifndef OGG_UTIL_H
#define OGG_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOL
#define BOOL unsigned char
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

//no error
#define OGG_RESULT_NO_ERROR                    0
//end of stream
#define OGG_RESULT_ERROR_READ_STREAM_EOFF      1
//error stream count
#define OGG_RESULT_ERROR_READ_STREAM_CNT       2
//error patterr OggS
#define OGG_RESULT_ERROR_CAPTURE_PATTERN       3


//continue page
#define OGG_PAGE_HEADER_TYPE_CONTINUE           0x01
//bein stream
#define OGG_PAGE_HEADER_TYPE_BEGIN_OF_STREAM    0x02
//end stream
#define OGG_PAGE_HEADER_TYPE_END_OF_STREAM      0x04


//pattern OggS
#define OGG_CAPTURE_PATTERN_ARR         {0x4F, 0x67, 0x57, 0x53}
#define OGG_CAPTURE_PATTERN_UINT        0x5367674F
#define OGG_CAPTURE_PATTERN_STR         "OggS"

//read or write stream
typedef void * ogg_stream_t;
//function result
typedef uint8_t ogg_result_t;


#pragma pack(push, 1)

typedef struct _ogg_page
{
    //0x5367674F or OggS
    uint32_t capture_pattern;
    //version
    uint8_t version;
    //0x02 fo first, 0x04 for last,  0x00 for other
    uint8_t header_type;
    //число, которое показывает сколько PCM сэмплов было закодировано за
    //все предыдущие страницы ПЛЮС данная страница
    //pcm samples count
    uint64_t granole_pos;
    //число, которое ОДИНАКОВОЕ для кажной страницы (пакета) для данного аудиострима.
    //и это число придумываете/генерируете вы
    //unique number
    uint32_t bitstream_sn;
    //OGG page number. for first = 0
    uint32_t number;
    //Для подсчета КС, нужно вписать нули на место КС и выделить всю страницу (1 Ogg-пакет)
    //control sum
    uint32_t checksum;
    //число сегментов в странице, диапазон значений [0-255] (0 и 255 допустимы)
    //segments count
    uint8_t segments_cnt;
    //таблица размеров сегментов в странице.
    //Количество записей в таблице определяется page_segment, в случае нулевого значения, segment_table отсутствует.
    //seegments sizes
    uint8_t segment_size[256];
} ogg_page_t;

#pragma pack(pop)

//returns the number of bytes read
typedef uint8_t ogg_stream_read_t(ogg_stream_t stream, uint8_t * data, uint8_t data_len);

typedef struct _ogg_decoder
{
  //read stream
  ogg_stream_t stream;
  //read function
  ogg_stream_read_t * stream_read;
  //current page
  ogg_page_t page;
  //current page is load
  BOOL page_load;
  // segment index
  uint8_t segment_index;

  //for statistics
#ifdef OGG_STATISTIC
  uint32_t stat_pages_cnt;
  uint32_t stat_segments_cnt;
  uint32_t stat_bytes_cnt;
#endif
} ogg_decoder_t;



//init decoder
void ogg_decoder_init(ogg_decoder_t * ogg_decoder, ogg_stream_t stream, ogg_stream_read_t * stream_read);
//read one page
ogg_result_t ogg_read_page_data(ogg_decoder_t * ogg_decoder, ogg_page_t * ogg_page);
//read one segment
ogg_result_t ogg_next_segment(ogg_decoder_t * ogg_decoder, uint8_t * segment, uint8_t * segment_size);


//returns the number of bytes write
typedef uint8_t ogg_stream_write_t(ogg_stream_t stream, uint8_t * data, uint16_t data_len);

//write one page
ogg_result_t ogg_write_page(ogg_stream_t stream, ogg_stream_write_t * ogg_stream_write, ogg_page_t * ogg_page, uint8_t segments_data[256][256]);


//------------------------- OPUS -----------------------------------------------



#pragma pack(push, 1)

//pattern OpusHead
#define OGG_OPUS_HEAD_SIGNATURE         "OpusHead"

typedef struct _opus_head_t
{
   //must be OpusHead
   char signature[8];
   //version 0x01 for this spec
   uint8_t version;
   //number of channels
   uint8_t channels_count;
   //Количество выборок, которые следует отбросить с начала трансляции
   //skip
   uint16_t pre_skip;
   //frequency
   uint32_t sample_rate;
   //
   int16_t output_gain;
   //Семейство сопоставлений каналов (8 бит без знака)
   //-- 0 = один поток: моно или стерео L,R
   // -- 1 = каналы в порядке спецификации vorbis: моно или L,R стерео или ... или FL,C,FR,RL,RR,LFE, ...
   // -- 2..254 = зарезервировано (трактуется как 255)
   //-- 255 = нет определенного значения канала
   //Если семейство сопоставлений каналов > 0
   //- Число потоков 'N' (8 бит без знака): ДОЛЖНО быть > 0.
   //- Счетчик двухканального потока «M» (8 бит без знака): ДОЛЖЕН удовлетворять M <= N, M+N <= 255.
   // - Отображение каналов (8*c бит)
   //-- один индекс потока (8 бит без знака) на канал (255 означает молчание во всем файле)
   uint8_t channel_map;
} opus_head_t;


//pattern OpusTags
#define OGG_OPUS_TAGS_SIGNATURE         "OpusTags"


typedef struct _opus_tags_t
{
   //must be OpusTags
   char signature[8];

   uint8_t data[248];

} opus_tags_t;


#pragma pack(pop)

uint8_t * add_opus_tag(uint8_t * opus_data, uint8_t opus_data_size, char * tag);

uint8_t * get_opus_tag(uint8_t * data, char * buf, uint8_t buf_size);


#ifdef __cplusplus
}
#endif

#endif
