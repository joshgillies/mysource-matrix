<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright © Squiz Pty Ltd      |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: persistent_cookie_manager.inc,v 1.2 2006/12/07 00:07:09 emcdonald Exp $
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
* @version $Revision: 1.2 $
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
