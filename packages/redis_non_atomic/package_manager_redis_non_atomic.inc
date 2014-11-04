<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Redis_Non_Atomic
*
* Purpose
*    Allows using Redis storage through twemproxy (https://github.com/twitter/twemproxy)
*    Twemproxy doesn't support the Redis multi command so we just
*    avoid that
*
* @package MySource_Matrix_Packages
* @subpackage marketo
*/
class Package_Manager_Redis_Non_Atomic extends Package_Manager
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/redis_non_atomic';
		$this->Package_Manager();

	}//end constructor


}//end class

