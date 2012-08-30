<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: persistent_cookie_manager.inc,v 1.5 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.5 $
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
