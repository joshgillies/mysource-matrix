<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: locking_method_redis_edit_fns.inc,v 1.2 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/locking/locking_method/locking_method_edit_fns.inc';


/**
* Locking_Method_Redis_Edit_Fns
*
* Purpose
*
*
* @author		Chris Smith <csmith@squiz.net>
* @version		$Revision: 1.2 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method_Redis_Edit_Fns extends Locking_Method_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	public function __construct()
	{
		parent::__construct();

	}//end __construct()


}//end class

?>
