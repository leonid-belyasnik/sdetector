/**
* \file		common.h
* \brief	Common data definitions.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		12/05/2017
*/

/**
* \defgroup	COMMONDATA Common data.
* \brief	Common data definitions.
*
*  @{
*/
/** scanner port */
#define SVRPORT	"9999"
/** scanner host */
#define SVRHOST "localhost"
#ifdef _MSC_VER
#define MLFILE "malware_list.txt"
#define TESTFILE "test.dat"
#else
#define MLFILE "/tmp/malware_list.txt"
#define TESTFILE "/tmp/test.dat"
#endif
/** @} */