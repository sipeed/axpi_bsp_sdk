#ifndef TINYALSA_ATTRIBUTES_H
#define TINYALSA_ATTRIBUTES_H

/** @defgroup libtinyalsa-attributes
 * @brief GCC attributes to issue diagnostics
 * when the library is being used incorrectly.
 * */

#ifdef __GNUC__

/** Issues a warning when a function is being
 * used that is now deprecated.
 * @ingroup libtinyalsa-attributes
 * */
#define TINYALSA_DEPRECATED __attribute__((deprecated))

/** Issues a warning when a return code of
 * a function is not checked.
 * @ingroup libtinyalsa-attributes
 * */
#define TINYALSA_WARN_UNUSED_RESULT __attribute__((warn_unused_result))

#else /* __GNUC__ */

/** This is just a placeholder for compilers
 * that aren't GCC or Clang.
 * @ingroup libtinyalsa-attributes
 * */
#define TINYALSA_DEPRECATED

/** This is just a placeholder for compilers
 * that aren't GCC or Clang.
 * @ingroup libtinyalsa-attributes
 * */
#define TINYALSA_WARN_UNUSED_RESULT

#endif /* __GNUC__ */

#endif /* TINYALSA_ATTRIBUTES_H */
