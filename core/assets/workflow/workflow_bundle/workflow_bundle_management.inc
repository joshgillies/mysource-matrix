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
* $Id: workflow_bundle_management.inc,v 1.3 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Bundle_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Bundle_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager installing this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'						=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						// Messages for each stage in the workflow process
						'start_workflow_subject'	=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> 'Workflow Approval Required',
													   ),
						'progress_workflow_subject'	=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> 'Workflow Approval Required',
													   ),
						'approve_workflow_subject'	=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> 'Workflow Approved',
													   ),
						'reject_workflow_subject'	=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> 'Workflow Rejected',
													   ),
						// Messages for each stage in the workflow process
						'start_workflow_msg'		=> Array(
														'added'		=> '0.1',
														'type'		=> 'wysiwyg',
														'default'	=> '',
													   ),
						'progress_workflow_msg'		=> Array(
														'added'		=> '0.1',
														'type'		=> 'wysiwyg',
														'default'	=> '',
													   ),
						'approve_workflow_msg'		=> Array(
														'added'		=> '0.1',
														'type'		=> 'wysiwyg',
														'default'	=> '',
													   ),
						'reject_workflow_msg'		=> Array(
														'added'		=> '0.1',
														'type'		=> 'wysiwyg',
														'default'	=> '',
													   ),
					  );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'AssetStatusUpdate',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'minor',
														'indirect'		=> FALSE,
														'global'		=> FALSE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> NULL,
														'value'			=> '',
														'link_type'		=> SQ_LINK_NOTICE,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>