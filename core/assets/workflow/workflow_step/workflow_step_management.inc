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
* $Id: workflow_step_management.inc,v 1.15 2009/04/06 05:05:17 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

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
						'message_invitation'					=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when this workflow step begins',
																   ),
						'message_approve'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when changes to an asset in workflow are approved by a user',
																   ),
						'message_reject'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when changes to an asset in workflow are rejected by a user',
																   ),
						'message_stale'							=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when the workflow passes the expiry time, but is not escalated',
																   ),
						'message_escalated'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when the workflow is escalated from this step',
																   ),
					  );

	}//end constructor


}//end class

?>
