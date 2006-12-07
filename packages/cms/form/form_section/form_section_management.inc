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
* $Id: form_section_management.inc,v 1.15 2006/12/07 05:42:55 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Section_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Section_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'sort_order'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'questions'				=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'question_col_width'	=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '40%',
												   ),
						'answer_col_width'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '60%',
												   ),
						'section_width'			=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'html_width',
													'default'	=> '100%',
												   ),
						'next_questionid'		=> Array(
													'added'		=> '0.3',
													'type'		=> 'int',
													'default'	=> 0,
												   ),
					  );

		}//end constructor


}//end class
?>
