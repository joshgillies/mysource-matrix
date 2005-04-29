<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: poll_question_management.inc,v 1.3 2005/04/29 05:40:31 gsherwood Exp $
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
* @version $Revision: 1.3 $
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
				'name' 			=> Array(
							 'added' => '0.1',
							 'type'  => 'text',
							 'default' => ''
							 ),
				'options'		=> Array(
							 'added' => '0.1',
							 'type'  => 'serialise',
							 'default'	=> Array()
							 ),
				'voters'		=> Array(
							 'added' => '0.1',
							 'type'  => 'serialise',
							 'default'	=> Array()
							 )
				);

	}//end constructor


}//end class

?>