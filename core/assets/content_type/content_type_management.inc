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
* $Id: content_type_management.inc,v 1.9 2009/11/27 05:37:14 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'			=> '0.0.1',
									'type'			=> 'text',
									'default'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
						'html'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_contextable' 	=> TRUE,
								   ),
					  );

	}//end constructor


}//end class
?>
