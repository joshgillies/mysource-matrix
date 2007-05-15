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
* $Id: online_quiz_question_group_edit_fns.inc,v 1.1 2007/05/15 03:39:42 mbrydon Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Online Quiz Question Group
*
* Purpose
*	A group to house Online Quiz Questions
*
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Group_Edit_Fns extends Folder_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Group_Edit_Fns()
	{
		$this->Folder_Edit_Fns();

		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


}//end class
?>
