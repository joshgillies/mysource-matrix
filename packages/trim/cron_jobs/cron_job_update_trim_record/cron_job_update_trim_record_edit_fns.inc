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
* $Id: cron_job_update_trim_record_edit_fns.inc,v 1.1.1.1 2008/07/04 04:56:22 hnguyen Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Update_Trim_Record_Edit_Fns 
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Update_Trim_Record_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Update_Trim_Record_Edit_Fns()
	{
		parent::__construct();

	}//end constructor



    /**
    * Process the tasks interface
    *
    * @param Cron_Job_Clear_Matrix_Cache    $asset  the owning asset
    * @param Backend_Outputter              $o      the backend outputter
    * @param string                         $prefix prefix for form elements
    *
    * @return boolean
    * @access public
    */
    public function processInLineBackend(Cron_Job_Packaging_Trim_Record $asset, Backend_Outputter $o, $prefix)
    {
        $when = $this->_processWhenBox($asset, $o, $prefix);
        if ($when) {
            return $asset->setAttrValue('when', $when);
        } else {
            return FALSE;
        }

    }//end processInLineBackend()


}//end class

?>
