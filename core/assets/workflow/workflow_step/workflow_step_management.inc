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
* $Id: workflow_step_management.inc,v 1.13 2006/12/05 05:50:00 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Workflow_Step_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'									=> Array(
																	'default'	=> '',
																   ),
						'logic'									=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'text',
																	'default'	=> 'all',
																   ),
						'logic_num'								=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'int',
																	'default'	=> '0',
																   ),
						'expiry_time'							=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'duration',
																	'default'	=> '0',
																   ),
						'escalate'								=> Array(
																	'added'			=> '0.3',
																	'type'			=> 'boolean',
																	'default'		=> 'false',
																	'parameters'	=> Array(
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
						'require_comment_from_unlisted_users'	=> Array(
																	'added'			=> '0.2',
																	'type'			=> 'boolean',
																	'default'		=> '0',
																	'parameters'	=> Array(
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
					  );

	}//end constructor


}//end class

?>