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
* $Id: online_quiz_question_group_management.inc,v 1.3 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online Quiz Question Group Management
*
* @see Asset_Management
*
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Online_Quiz_Question_Group_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'score_categories'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Score-specific categories',
											   ),
					  );

	}//end constructor


}//end class
?>
