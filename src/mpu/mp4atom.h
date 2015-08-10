#ifndef NET_MMT_MPU_MP4ATOM_H_
#define NET_MMT_MPU_MP4ATOM_H_

#include <stdint.h>

typedef struct uuid_s{
    uint8_t b[16];
} uuid_t;

#define MP4_FOURCC( a, b, c, d ) \
    (((uint32_t)a) | (((uint32_t)b)<<8) | (((uint32_t)c)<<16) | (((uint32_t)d)<<24))


#define ATOM_root MP4_FOURCC( 'r', 'o', 'o', 't' )

#define ATOM_ftyp MP4_FOURCC( 'f', 't', 'y', 'p' )
#define ATOM_uuid MP4_FOURCC( 'u', 'u', 'i', 'd' )
#define ATOM_mmpu MP4_FOURCC( 'm', 'm', 'p', 'u' )
#define ATOM_moov MP4_FOURCC( 'm', 'o', 'o', 'v' )
#define ATOM_moof MP4_FOURCC( 'm', 'o', 'o', 'f' )
#define ATOM_mdat MP4_FOURCC( 'm', 'd', 'a', 't' )

// pps/sps: moov -> trak -> mdia -> minf -> stbl -> stsd -> avcl -> avcC
#define ATOM_trak MP4_FOURCC( 't', 'r', 'a', 'k' )
#define ATOM_mdia MP4_FOURCC( 'm', 'd', 'i', 'a' )
#define ATOM_minf MP4_FOURCC( 'm', 'i', 'n', 'f' )
#define ATOM_stbl MP4_FOURCC( 's', 't', 'b', 'l' )
#define ATOM_stsd MP4_FOURCC( 's', 't', 's', 'd' )
#define ATOM_avc1 MP4_FOURCC( 'a', 'v', 'c', '1' )
#define ATOM_avcC MP4_FOURCC( 'a', 'v', 'c', 'C' )

// moof


typedef struct atom_ftyp_s {
    uint32_t major_brand;
    uint32_t minor_version;
    uint32_t compatible_brands_count;
    uint32_t *compatible_brands;
} atom_ftyp_t;

typedef struct atom_mmpu_s {
    uint8_t  version;
    uint32_t flags;
    uint8_t  is_complete;
    uint8_t  reserved;
    uint32_t mpu_sequence_number;
    uint32_t asset_id_scheme;
    uint32_t asset_id_length;
    char *asset_id_value;
} atom_mmpu_t;

typedef struct atom_tfdt_s {
	uint8_t  version;
	uint32_t flags;
    uint64_t baseMediaDecodeTime;
} atom_tfdt_t;

typedef struct atom_mvhd_s {
   uint8_t  version;
   uint32_t flags;

   uint64_t creation_time;
   uint64_t modification_time;
   uint32_t timescale;
   uint64_t duration;

   int32_t  rate;
   int16_t  volume;
   int16_t  reserved1;
   uint32_t reserved2[2];
   int32_t  matrix[9];
   uint32_t predefined[6];
   uint32_t next_track_id;
} atom_mvhd_t;

typedef struct atom_mfhd_s {
    uint32_t sequence_number;
    uint8_t *vendor_extension;
} atom_mfhd_t;

typedef struct atom_tfra_s
{
   uint8_t  version;
   uint32_t flags;

   uint32_t track_ID;
   uint32_t number_of_entries;

   uint8_t length_size_of_traf_num;
   uint8_t length_size_of_trun_num;
   uint8_t length_size_of_sample_num;

   uint32_t *time;
   uint32_t *moof_offset;
   uint8_t *traf_number;
   uint8_t *trun_number;
   uint8_t *sample_number;
} atom_tfra_t;

typedef struct atom_avcC_s {
   uint8_t version;
   uint8_t profile;
   uint8_t profile_compatibility;
   uint8_t level;

   uint8_t reserved1;     /* 6 bits */
   uint8_t length_size;

   uint8_t reserved2;    /* 3 bits */
   uint8_t  sps;
   uint16_t *sps_length;
   uint8_t  **p_sps;

   uint8_t  pps;
   uint16_t *pps_length;
   uint8_t  **p_pps;

   /* XXX: Hack raw avcC atom payload */
   int     avcC;
   uint8_t *p_avcC;
} atom_avcC_t;


// common atoms
//  a) ftyp
//  b) avcC in moov/trak/mdia/minf/stbl/stsd/avcl
//  c) mdat
// common containers: 
//  a) moov/trak/mdia/minf/stbl/stsd/avcl
//  b) moof/

// for some atom containers
typedef unsigned char* data_t;
typedef struct atom_info_s {
    int     pos;
    int     type;
    uuid_t  uuid;
    char    name[5]; // box name: 4chars + '\0'
    int     size;
    data_t  data;
}atom_info_t;

typedef struct mp4_root_s {
    atom_info_t ftyp; // required
    atom_info_t mmpu; // required
    atom_info_t moov; // required
    atom_info_t moof; // the first moof
    atom_info_t mdat;
    int     size;
    data_t  data;
}mp4_root_t;

#endif // NET_MMT_MPU_MP4ATOM_H_
