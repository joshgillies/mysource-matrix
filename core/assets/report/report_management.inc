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
* $Id: report_management.inc,v 1.10 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Report_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['name']['added']   = '0.2';
		$this->vars['name']['type']    = 'text';
		$this->vars['name']['default'] = '';

		$this->vars['generated_date']['added']   = '0.1';
		$this->vars['generated_date']['type']    = 'text';
		$this->vars['generated_date']['default'] = '-- '.translate('Never').' --';


		$this->vars['generated_user']['added']   = '0.1';
		$this->vars['generated_user']['type']    = 'text';
		$this->vars['generated_user']['default'] = '';

		$this->vars['html']['added']       = '0.3';
		$this->vars['html']['type']        = 'text';
		$this->vars['html']['default']     = 'Front end component has not been generated for this report';
		$this->vars['html']['description'] = 'This is a default location of the generated report.';

	}//end constructor


}//end class

?>