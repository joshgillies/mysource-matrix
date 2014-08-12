<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
*/


/**
* General Security Functions
*
* Purpose
*	  Security functions that are useful, but don't fall into any other category
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @package Fudge
* @subpackage Security
*/

/**
* Hash a password.
*
* @param string		$password	The password to hash.
* @param string		$prefix		The password algorithm prefix.
* @param mixed		$options	The hashing options.
*
* @return string
* @access public
*/
function security_password_hash($password, $prefix=NULL, array $options=NULL)
{
	if ($prefix === NULL) {
		$prefix = security_get_default_algorithm();
	}

	if ($options === NULL || is_array($options) === FALSE) {
		$options = security_get_default_options();
	}

	$salt	= array_get_index($options, 'salt', security_generate_salt());
	$rounds = array_get_index($options, 'rounds', 5000);
	switch ($prefix) {
		case '2y':
			if (security_is_algorithm_available('2y') === TRUE) {
				if (version_compare(PHP_VERSION, '5.5.0') < 0) {
					// Only include this file if under PHP 5.5.
					include_once SQ_FUDGE_PATH.'/general/password.inc';
				}//end if

				// Remap rounds to cost.
				if (isset($options['rounds']) === TRUE && isset($options['cost']) === FALSE) {
					$options['cost'] = $options['rounds'];
				}

				if (isset($options['cost']) === TRUE) {
					$cost = $options['cost'];
					if ($cost < 4 || $cost > 31) {
						$options['cost'] = 10;
					}
				}

				if (isset($options['salt']) === TRUE) {
					if (strlen($options['salt']) < 22) {
						// Salt is too short, do not use it.
						unset($options['salt']);
					}
				}

				$hash = password_hash($password, PASSWORD_BCRYPT, $options);
				if ($hash !== FALSE) {
					break;
				}
			}//end if

		case '6':
			if (security_is_algorithm_available('6') === TRUE) {
				// No bcrypt present, but crypt() can handle SHA512.
				if (strlen($salt) > 16) {
					$salt	= substr($salt, 0, 16);
				}

				if ($rounds < 1000	|| $rounds > 5000) {
					$rounds = 5000;
				}

				$hash	= crypt($password, '$6$rounds='.$rounds.'$'.$salt.'$');
				break;
			}//end if

		case '6o':
			if (security_is_algorithm_available('6o') === TRUE) {
				// crypt() cannot handle sha512, so use hash instead.
				if (strlen($salt) > 16) {
					$salt	= substr($salt, 0, 16);
				}

				if ($rounds < 1000	|| $rounds > 5000) {
					$rounds = 5000;
				}

				$hash	= '$6o$rounds='.$rounds.'$'.$salt.'$';
				$temp	= hash('sha512', $password.$salt);
				for ($i = 0; $i < $rounds; $i++) {
					$temp = hash('sha512', $temp.$salt);
				}

				$hash .= $temp;
				break;
			}//end if

		case '1':
		default:
			// Fallback mode: MD5. This should never occur if PHP is installed correctly.
			if (strlen($salt) > 12) {
				$salt = substr($salt, 0, 12);
			}

			$hash = crypt($password, '$1$'.$salt.'$');
		break;
	}//end switch

	return $hash;

}//end security_password_hash()


/**
* Verify a password.
*
* @param string		$password	The password to verify.
* @param string		$prefix		The password algorithm prefix.
* @param string		$hash		The current hash.
*
* @return boolean
* @access public
*/
function security_password_verify($password, $hash)
{
	$verified = FALSE;
	if (strpos($hash, '$2y$') === 0) {
		if (version_compare(PHP_VERSION, '5.5.0') < 0) {
			// Only include this file if under PHP 5.5.
			include_once SQ_FUDGE_PATH.'/general/password.inc';
		}//end if

		$verified = password_verify($password, $hash);
	} else if (strpos($hash, '$6$') === 0) {
		// SHA512, but crypt can handle it.
		$test	  = crypt($password, $hash);
		$verified = ($test === $hash);
	} else if (strpos($hash, '$6o$') === 0) {
		// SHA512, but the hash method.
		$options  = array_get_index(security_get_password_info($hash), 'options', array());
		$test	  = security_password_hash($password, '6o', $options);
		$verified = ($test === $hash);
	} else if (strpos($hash, '$1$') === 0) {
		// Lovely, a left over MD5.
		$test	  = crypt($password, $hash);
		$verified = ($test === $hash);
	}//end if

	return $verified;

}//end security_password_verify()


/**
* Returns TRUE, if a password needs a rehash.
*
* @param string		$hash		The password hash to check.
* @param string     $prefix     The password algorithm prefix.
* @param array		$options	The hashing options.
*
* @return boolean
* @access public
*/
function security_password_needs_rehash($hash, $prefix=NULL, array $options=array())
{
	if ($prefix === NULL) {
		$prefix = security_get_default_algorithm();
	}

	$rehash = FALSE;
	switch ($prefix) {
		case '2y':
			// Normal method.
			if (strpos($hash, '$2y$') !== 0) {
				$rehash = TRUE;
				break;
			} else {
				// Same hash, need to check the options.
				if (version_compare(PHP_VERSION, '5.5.0') < 0) {
					// Only include this file if under PHP 5.5.
					include_once SQ_FUDGE_PATH.'/general/password.inc';
				}//end if

				// Remap rounds to cost.
				if (isset($options['rounds']) === TRUE && isset($options['cost']) === FALSE) {
					$options['cost'] = $options['rounds'];
				}

				if (isset($options['cost']) === TRUE) {
					$cost = $options['cost'];
					if ($cost < 4 || $cost > 31) {
						$options['cost'] = 10;
					}
				}

				if (isset($options['salt']) === TRUE) {
					if (strlen($options['salt']) < 22) {
						// Salt is too short, do not use it.
						unset($options['salt']);
					}
				}

				$rehash = password_needs_rehash($hash, PASSWORD_BCRYPT, $options);
			}//end if
		break;

		case '6':
		case '6o':
		case '1':
			if (strpos($hash, '$'.$prefix.'$') !== 0) {
				$rehash = TRUE;
				break;
			} else {
				// Same hash, need to check the options.
				$info     = security_get_password_info($hash);
				$origRnd  = array_get_index($info['options'], 'rounds', NULL);
				$testRnd  = array_get_index($options, 'rounds', NULL);
				if ($origRnd !== $testRnd) {
					$rehash = TRUE;
				}
			}
		break;

		default:
			// Unknown option, rehash.
			$rehash = TRUE;
		break;
	}//end switch

	return $rehash;

}//end security_password_needs_rehash()


/**
 * Returns TRUE, if the algorithm is available.
 *
 * @param string $prefix The algorithm prefix to check.
 *
 * @returns boolean
 */
function security_is_algorithm_available($prefix)
{
	$available = FALSE;
	switch ($prefix) {
		case '2y':
			if (function_exists('crypt') === TRUE) {
				$bcrypt = '$2y$04$usesomesillystringfore7hnbRJHxXVLeakoG8K30oukPsA.ztMG';
				$test	= crypt("password", $bcrypt);
				if ($test === $bcrypt) {
					$available = TRUE;
				}
			}
		break;

		case '6':
			if (function_exists('crypt') === TRUE) {
				$sha512 = '$6$rounds=5000$usesomesillystri$nnCrG0XcyKwkRXepV1dRXhqEhP0r2sdjV8bt5gCcljMzCikm9bUX/7p3XFtKdxi5sTwUISBZHwcTXhwXYM/rl1';
				$test	= crypt("password", $sha512);
				if ($test === $sha512) {
					$available = TRUE;
				}
			}
		break;

		case '6o':
			if (function_exists('hash') === TRUE) {
				$sha512 = 'b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb980b1d7785e5976ec049b46df5f1326af5a2ea6d103fd07c95385ffab0cacbc86';
				$test	= hash('sha512', 'password');
				if ($test === $sha512) {
					$available = TRUE;
				}
			}
		break;

		case '1':
		default:
			if (function_exists('crypt') === TRUE) {
				$available = TRUE;
			}
		break;
	}//end switch

	return $available;

}//end security_is_algorithm_available()


/**
 * Return the default hashing algorithm.
 *
 * @return integer
 */
function security_get_default_algorithm()
{
	$bcrypt = '$2y$04$usesomesillystringfore7hnbRJHxXVLeakoG8K30oukPsA.ztMG';
	$sha512 = '$6$rounds=5000$usesomesillystri$nnCrG0XcyKwkRXepV1dRXhqEhP0r2sdjV8bt5gCcljMzCikm9bUX/7p3XFtKdxi5sTwUISBZHwcTXhwXYM/rl1';
	$test	= crypt("password", $bcrypt);
	$prefix	= NULL;
	if ($test === $bcrypt) {
		$prefix = '2y';
	} else if ($test === $sha512) {
		$prefix = '6';
	} else {
		if (function_exists('hash') === TRUE) {
			$sha512 = 'b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb980b1d7785e5976ec049b46df5f1326af5a2ea6d103fd07c95385ffab0cacbc86';
			$test	= hash('sha512', 'password');
			if ($test === $sha512) {
				$prefix = '6o';
			}
		}

		if ($prefix === NULL) {
			// Ran out of options.
			$prefix = '1';
		}
	}//end if

	return $prefix;

}//end security_get_default_algorithm()


/**
 * Return the default options based on the default algorithm.
 *
 * @return array
 */
function security_get_default_options()
{
	switch (security_get_default_algorithm()) {
		case '2y':
			$options = array('rounds' => 10);
		break;

		case '6':
		case '6o':
			$options = array('rounds' => 5000);
		break;

		case '1':
		default:
			$options = array();
		break;
	}//end switch

	return $options;

}//end security_get_default_options()


/**
 * Generate a random salt.
 *
 * @return string
 */
function security_generate_salt()
{
	$buffer		  = '';
	$buffer_valid = false;
	$raw_salt_len = 30;
	if (function_exists('mcrypt_create_iv') && !defined('PHALANGER')) {
		$buffer = mcrypt_create_iv($raw_salt_len, MCRYPT_DEV_URANDOM);
		if ($buffer) {
			$buffer_valid = true;
		}
	}

	if (!$buffer_valid && function_exists('openssl_random_pseudo_bytes')) {
		$buffer = openssl_random_pseudo_bytes($raw_salt_len);
		if ($buffer) {
			$buffer_valid = true;
		}
	}

	if (!$buffer_valid && is_readable('/dev/urandom')) {
		$f = fopen('/dev/urandom', 'r');
		$read = strlen($buffer);
		while ($read < $raw_salt_len) {
			$buffer .= fread($f, $raw_salt_len - $read);
			$read = strlen($buffer);
		}
		fclose($f);
		if ($read >= $raw_salt_len) {
			$buffer_valid = true;
		}
	}

	if (!$buffer_valid || strlen($buffer) < $raw_salt_len) {
		$bl = strlen($buffer);
		for ($i = 0; $i < $raw_salt_len; $i++) {
			if ($i < $bl) {
				$buffer[$i] = $buffer[$i] ^ chr(mt_rand(0, 255));
			} else {
				$buffer .= chr(mt_rand(0, 255));
			}
		}
	}

	$salt = str_replace('+', '.', base64_encode($buffer));

	return $salt;

}//end security_generate_salt()


/**
 * Return the options from a hash.
 *
 * @param string $hash The hash to process.
 *
 * @return array
 */
function security_get_password_info($hash)
{
	$info = array();

	if (strpos($hash, '$2y$') === 0) {
		// Normal method.
		if (version_compare(PHP_VERSION, '5.5.0') < 0) {
			// Only include this file if under PHP 5.5.
			include_once SQ_FUDGE_PATH.'/general/password.inc';
		}//end if

		$info		  = password_get_info($hash);
		$info['prefix'] = '2y';
	} else if (strpos($hash, '$6$') === 0) {
		// SHA512, but crypt method.
		$info['prefix']	  = '6';
		$info['algoName'] = 'SHA512';
		$info['options']  = array();

		$hash	= ltrim($hash, '$');
		$parts	= explode('$', $hash);
		$temp	= array_shift($parts);
		$rounds = array_shift($parts);
		if (strpos($rounds, 'rounds=') === 0) {
			$info['options']['rounds'] = (int) str_replace('rounds=', '', $rounds);
		} else {
			array_unshift($parts, $rounds);
		}

		$salt = array_shift($parts);
		if (empty($parts) === FALSE) {
			$info['options']['salt'] = $salt;
		}
	} else if (strpos($hash, '$6o$') === 0) {
		// SHA512, but hash method.
		$info['prefix']	  = '6o';
		$info['algoName'] = 'SHA512';
		$info['options']  = array();

		$hash	= ltrim($hash, '$');
		$parts	= explode('$', $hash);
		$temp	= array_shift($parts);
		$rounds = array_shift($parts);
		if (strpos($rounds, 'rounds=') === 0) {
			$info['options']['rounds'] = (int) str_replace('rounds=', '', $rounds);
		} else {
			array_unshift($parts, $rounds);
		}

		$salt = array_shift($parts);
		if (empty($parts) === FALSE) {
			$info['options']['salt'] = $salt;
		}
	} else if (strpos($hash, '$1$') === 0) {
		// Worst case: MD5.
		$info['prefix']	  = '1';
		$info['algoName'] = 'MD5';
		$info['options']  = array();

		$hash	= ltrim($hash, '$');
		$parts	= explode('$', $hash);
		$temp	= array_shift($parts);
		$salt	= array_shift($parts);
		if (empty($parts) === FALSE) {
			$info['options']['salt'] = $salt;
		}
	}//end if

	return $info;


}//end security_get_password_info()


/**
 * Regenerate the session id safely.
 *
 * There is a bug in PHP <5.5.3, where session_regenerate_id()
 * cannot prevent a session fixation attack when multiple cookies
 * are sent to the user's browser. This can lead to DOS attacks
 * in IE and Chrome. To fix, expire ALL the cookies for all paths
 * and subdomains. In PHP 5.5.3+, turn on strict sessions.
 *
 * @param boolean $delete_old If TRUE, delete the old session file.
 *
 * @return boolean
 */
function security_regenerate_session_id($delete_old=FALSE)
{
    $params  = session_get_cookie_params();
    $host    = array_get_index($_SERVER, 'HTTP_HOST', NULL);
    $pathPts = array();
    $hostPts = array();
    if ($params['path'] !== '/') {
        $pathPts = explode('/', $params['path']);
    }

    if ($host !== NULL) {
        $hostPts = explode('.', $host);
    }

    if (empty($pathPts) === FALSE) {
        $pathLn = count($pathPts);
        while ($pathLn >= 1) {
            $path = implode('/', $pathPts);
            security_send_reset_cookies($path, $host);

            array_pop($pathPts);
            $pathLn = count($pathPts);
        }
    } else {
        // Single path, just do the combination of hosts.
        security_send_reset_cookies($params['path'], $host);
    }

    $status = session_regenerate_id($delete_old);

    return $status;

}//end security_regenerate_session_id()


/**
 * Wrapper function to cycle thru the variation of hosts.
 *
 * @param string $path The current path.
 * @param string $host The full host name.
 *
 * @return void
 */
function security_send_reset_cookies($path, $host)
{
    $hostPts = array();
    // 1st Jan 1970
    $expiry  = 'Thu, 01 Jan 1970 00:00:00 GMT';
    if ($host !== NULL || ip2long($host) === FALSE) {
        // We are looking at a qualified domain, so break it down to cycle
        // through each iteration of subdomains.
        $hostPts = explode('.', $host);
    }

    if (empty($path) === TRUE) {
        $path = '/';
    }

    header('Set-Cookie: SQ_SYSTEM_SESSION=destroy; Expires='.$expiry.'; path='.$path, FALSE);
    if (empty($hostPts) === FALSE) {
        // Unfortunately, since IE and Chrome are lame browsers,
        // a DOS attack cookie could happen on qualified TLD domains.
        // eg. com.au, co.jp, co.uk etc.
        $hostLn = count($hostPts);
        while ($hostLn >= 2) {
            $hostname = implode('.', $hostPts);
            header('Set-Cookie: SQ_SYSTEM_SESSION=destroy; expires='.$expiry.'; domain='.$hostname.'; path='.$path, FALSE);

            array_shift($hostPts);
            $hostLn = count($hostPts);
        }
    }

}//end security_send_reset_cookies()


/**
 * Secure replacement for rand() and mt_rand().
 *
 * @param integer $min The minimum value.
 * @param integer $max The maximum value.
 *
 * @return integer
 */
function security_rand($min=0, $max=2147483647)
{
    if ($min < 0) $min = 0;
    $value = -1;
    do {
        if (function_exists('openssl_random_pseudo_bytes') === TRUE) {
            $entropy = TRUE;
            $rand    = openssl_random_pseudo_bytes(4, $entropy);
        } else {
            $file = fopen('/dev/random', 'rb');
            if (function_exists('stream_set_read_buffer') === TRUE) {
                stream_set_read_buffer($file, 0);
            }

            $rand = fread($file, 4);
            fclose($file);
        }

        $value   = floor($max * (hexdec(bin2hex($rand)) / 0xffffffff));
    } while($value < $min || $value > $max);

    return $value;

}//end security_rand()

?>
