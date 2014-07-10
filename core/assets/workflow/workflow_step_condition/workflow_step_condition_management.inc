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
* $Id: workflow_step_condition_management.inc,v 1.16 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Condition_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Condition_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'userid'			=> Array (
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> 0,
											   ),
						'logic'				=> Array (
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '>=',
											   ),
						'logic_num'			=> Array (
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> 1,
											   ),
						'notify'			=> Array (
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'require_comment'	=> Array (
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'dynamic_userid'	=> Array (
												'added'			=> '0.2',
												'type'			=> 'text',
												'default'		=> '',
											   ),

					  );

	}//end constructor


}//end class

?>