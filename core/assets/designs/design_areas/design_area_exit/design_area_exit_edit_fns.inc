<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_exit_edit_fns.inc,v 1.1 2010/05/05 01:39:50 hnguyen Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Exit_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Exit_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->static_screens['details']['force_unlock'] = FALSE;
		parent::__construct();

	}//end constructor


}//end class
?>
