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
* $Id: content_type_wysiwyg_management.inc,v 1.14 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_WYSIWYG_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_WYSIWYG_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'htmltidy_status'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> 'wait',
												'description'	=> 'Status of the last html tidy clean up action. (wait,pass,fail)',
												'is_contextable' => TRUE,
											   ),
						'htmltidy_errors'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'text',
												'default'		=> 'No error found',
												'description'	=> 'Status of the last html tidy clean up action. (wait,pass,fail)',
												'is_contextable' => TRUE,
											   ),
					  );


	}//end constructor


}//end class

?>
