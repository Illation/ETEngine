# Package data format

## Overview

Packages for EtEngine work similar to zip files, in that they have a "central directory" dictionary that lists the offsets of all files in the 
package, and then the file content itself.

Files in a package have their own headers with meta information followed by data, and are compressed individually 
(as opposed to the entire package being compressed).
This allows the engine to only decompress files that need to be loaded without decompressing the entire archive.

The difference from zip is that the central directory is at the beginning and all unnecessary meta info is stripped from the format.


## Data layout

### Package

| Package |  | 
| ------- | ------ | 
| num_entries (x) | 64 |
| Central_Dir_File_Info * x | x * 128 |
| Package_File * x | remaining data in package |


### Central Directory File Info

| Central_Dir_File_Info |  | 
| ------- | ------ | 
| file_id | 64 |
| offset | 64 |


### Package File

| Package_File |  | 
| ------- | ------ | 
| file_id | 64 |
| Compression_Type | 8 |
| name_length (x) | 16 |
| size (y) | 64 |
| name | 8 * x |
| content | 8 * y |


### Compression Type

| Compression_Type |  | 
| ------- | ------ | 
| 0 | Store - No compression |
| 1 | Reserved for Brotli |
