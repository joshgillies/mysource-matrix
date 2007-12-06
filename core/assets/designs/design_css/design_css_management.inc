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
* $Id: design_css_management.inc,v 1.12 2007/12/06 04:28:50 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['id_name'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								 );
		$this->vars['cache_css'] = Array(
									'added'			=> '0.0.1',
									'type'			=> 'boolean',
									'default'		=> FALSE,
									'parameters'	=> Array('allow_empty' => FALSE),
								   );
		$this->vars['strip_whitespace'] = Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array('allow_empty' => FALSE),
										  );

	}//end constructor


}//end class
?>
