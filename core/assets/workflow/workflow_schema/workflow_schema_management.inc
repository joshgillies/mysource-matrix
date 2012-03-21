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
* $Id: workflow_schema_management.inc,v 1.16 2012/03/21 05:34:36 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Schema_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Schema_Management extends Asset_Management
{


	/**
	* Constructor
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
						'notify_starter_on_live'	=> Array (
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																			),
														'description'	=> 'If enabled, will send the Notify on Live message to the initiator of the workflow process.',
													   ),
						'subject_notify_on_live'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'text',
														'default'		=> 'Asset Made Live',
														'description'	=> 'Custom subject to users when workflow is complete and the asset is made live',
													   ),
						'message_notify_on_live'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'Custom message to users when workflow is complete and the asset is made live',
													   ),														   
					  );

	}//end constructor


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'css/workflow_schema_edit_fns.css',
				'js/workflow_schema_edit_fns.js',
				'files/wf_drag_handle.png',
			   );

	}//end _getFiles()


}//end class

?>
