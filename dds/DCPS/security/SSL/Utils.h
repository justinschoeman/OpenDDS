/*
 * Distributed under the DDS License.
 * See: http://www.DDS.org/license.html
 */

#ifndef OPENDDS_SECURITY_SSL_UTILS_H
#define OPENDDS_SECURITY_SSL_UTILS_H

#include "Certificate.h"
#include "PrivateKey.h"
#include "dds/DdsDcpsGuidC.h"
#include "dds/DdsDcpsCoreC.h"
#include <string>

namespace OpenDDS {
namespace Security {
namespace SSL {

  enum URI_SCHEME
  {
    URI_UNKNOWN,
    URI_FILE,
    URI_DATA,
    URI_PKCS11,
  };

  URI_SCHEME extract_uri_info(const std::string& uri, std::string& path);

  DdsSecurity_Export int make_adjusted_guid(const OpenDDS::DCPS::GUID_t& src,
                                            OpenDDS::DCPS::GUID_t& dst,
                                            const Certificate& target);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int make_nonce_256(std::vector<unsigned char>& nonce);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int make_nonce_256(DDS::OctetSeq& nonce);

  /// Gets byte from array as though it were shifted right one bit
  DdsSecurity_Export unsigned char offset_1bit(const unsigned char array[],
                                               size_t i);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int hash(const std::vector<const DDS::OctetSeq*>& src,
                              DDS::OctetSeq& dst);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int hash_serialized(const DDS::BinaryPropertySeq& src,
                                         DDS::OctetSeq& dst);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int sign_serialized(const DDS::BinaryPropertySeq& src,
                                         const PrivateKey& key,
                                         DDS::OctetSeq& dst);

  /**
   * @return int 0 on success; 1 on failure.
   */
  DdsSecurity_Export int verify_serialized(const DDS::BinaryPropertySeq& src,
                                           const Certificate& key,
                                           const DDS::OctetSeq& signed_data);

}  // namespace SSL
}  // namespace Security
}  // namespace OpenDDS

#endif
