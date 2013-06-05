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
* $Id: report_links_management.inc,v 1.10 2013/06/05 05:13:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Links_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Links_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->vars['display_options'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(
															'internal_section'	=> TRUE,
															'internal_broken'	=> TRUE,
															'internal_working'	=> TRUE,
															'external_section'	=> TRUE,
															'external_broken'	=> TRUE,
															'external_working'	=> TRUE,
															'email_section'		=> TRUE,
															'email_broken'		=> TRUE,
															'email_working'		=> TRUE,
														   ),
										 );
		$this->vars['report_broken_links'] = Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											 );
		$this->vars['report_info'] = Array(
										'added'		=> '0.2',
										'type'		=> 'text',
										'default'	=> '',
									 );
		$this->vars['statuses'] = Array(
									'added'			=> '0.3',
									'type'			=> 'serialise',
									'default'		=> Array('16'),
									'description'	=> 'Statuses that are being listed.',
								  );
		$this->vars['follow_link_redirect'] = Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											 );
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
