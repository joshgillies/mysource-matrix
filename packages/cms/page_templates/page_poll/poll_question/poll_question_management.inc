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
* $Id: poll_question_management.inc,v 1.9 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Poll_Question_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Poll_Question_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Poll_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'is_admin'		=> FALSE,
												'is_contextable'=> TRUE,
											   ),
						'options'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'voters'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'allow_multiple'	=> Array(
												'added'		=> '0.2',
												'type'		=> 'boolean',
												'default'	=> FALSE,
											   ),
					  );

	}//end constructor


}//end class

?>
