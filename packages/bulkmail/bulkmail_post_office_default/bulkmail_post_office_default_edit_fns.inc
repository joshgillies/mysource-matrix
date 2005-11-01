<?php

require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';

/**
* Bulkmail_Post_Office_Default_Edit_Fns
*
* Purpose
*
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/


class Bulkmail_Post_Office_Default_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Post_Office_Default_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


}//end class
?>
