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
* $Id: workflow_step_management.inc,v 1.18 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18 $
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
						'message_review_invitation'				=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when this workflow step begins by the Up for Review status',
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
						'message_review_cancel'					=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> '',
																	'description'	=> 'Custom message to users when an asset in Up for Review workflow is cancelled by a user',
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
						'subject_invitation'					=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Workflow Approval Required',
																	'description'	=> 'Custom subject to users when this workflow step begins',
																   ),
						'subject_review_invitation'				=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Asset Up For Review',
																	'description'	=> 'Custom subject to users when this workflow step begins by the Up for Review status',
																   ),																   
						'subject_approve'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Asset Changes Approved',
																	'description'	=> 'Custom subject to users when changes to an asset in workflow are approved by a user',
																   ),
						'subject_reject'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Asset Changes Rejected',
																	'description'	=> 'Custom subject to users when changes to an asset in workflow are rejected by a user',
																   ),
						'subject_review_cancel'					=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Asset Review Cancelled',
																	'description'	=> 'Custom subject to users when an asset in Up for Review workflow is cancelled by a user',
																   ),																   
						'subject_stale'							=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Stale Workflow Step',
																	'description'	=> 'Custom subject to users when the workflow passes the expiry time, but is not escalated',
																   ),
						'subject_escalated'						=> Array(
																	'added'			=> '0.4',
																	'type'			=> 'text',
																	'default'		=> 'Workflow Escalated',
																	'description'	=> 'Custom subject to users when the workflow is escalated from this step',
																   ),
					  );

	}//end constructor


}//end class

?>
