<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: persistent_cookie_manager.inc,v 1.1.4.1 2006/09/25 05:56:44 arailean Exp $
*
*/

require_once 'HTTP/Client/CookieManager.php';


/**
* Persistent_Cookie_Manager
*
* A class to override the PEAR cookie manager (part of HTTP_Client package)
* Necessary because PEAR's class erases session cookies every time the object is serialised, which we don't want
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1.4.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Persistent_Cookie_Manager extends HTTP_Client_CookieManager
{


	/**
	* Magic serialization function
	*
	* We are overriding parent's function
	* don't want to use the __sleep function of PEAR Cookie Manager
	*
	* @return array
	* @access private
	*/
	function __sleep()
	{
		return Array('_cookies');

	}//end __sleep()


	/**
	* Magic unserialization function, purges expired cookies
	*
	* This function allows cookies with empty 'expires' field to exist beyond serialisation
	*
	* @return void
	* @access private
	*/
	function __wakeup()
	{
		foreach ($this->_cookies as $hash => $cookie) {
			if (!empty($cookie['expires']) && strtotime($cookie['expires']) < time()) {
				unset($this->_cookies[$hash]);
			}
		}

	}//end __wakeup()


	/**
	* Removes session cookies
	*
	* @return void
	* @access public
	*/
	function deleteSessionCookies()
	{
		foreach ($this->_cookies as $hash => $cookie) {
			if (empty($cookie['expires'])) {
				unset($this->_cookies[$hash]);
			}
		}

	}//end deleteSessionCookies()


}//end class

?>
