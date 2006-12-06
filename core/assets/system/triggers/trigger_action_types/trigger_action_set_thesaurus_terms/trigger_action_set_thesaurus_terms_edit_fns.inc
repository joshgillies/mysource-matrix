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
* $Id: trigger_action_set_thesaurus_terms_edit_fns.inc,v 1.2 2006/12/06 00:00:30 bcaldwell Exp $
*
*/


/**
* Trigger_Action_Set_Thesaurus_Terms_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Thesaurus_Terms_Edit_Fns extends Trigger_Action_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Trigger_Action_Set_Thesaurus_Terms_Edit_Fns()
	{
		$this->Trigger_Action_Edit_Fns();

	}//end constructor


}//end class

?>
