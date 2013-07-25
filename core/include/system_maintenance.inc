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
* $Id: system_maintenance.inc,v 1.6 2013/07/25 23:23:50 lwright Exp $
*
*/


/**
* System Maintenance
*
* Purpose
*
*    A one-stop statistics shop for Matrix system information
*    useful for performance analysis and tuning.
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
class System_Maintenance extends MySource_Object
{
	/**
	* Manages display and processing of System Maintenance reports
	*
	* @param object	&$backend	Reference to backend object
	*
	* @return void
	* @access public
	*/
	public function paintBackend(&$backend)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		// if we dont have access, go away
		if (!$is_admin) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset'));
			exit();
		}

		$o =& $backend->out;

		if (isset($_GET['report_type_code'])) {
			$report = $_GET['report_type_code'];

			$this->_processReport($o, $report);
			$this->_paintReport($o, $report);
			exit;
		}

		$o->setHeading(translate('system_maintenance'), sq_get_icon($o->filesPath('/images/icons/header/system_maintenance.png'), 20, 20, translate('system_maintenance')));
		$o->setPageTitle(translate('system_maintenance'));

                $o->openSection(translate('reports_and_maintenance_tasks'));
                $o->openField('');
                        $reports = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('simple_report');

                        if (count($reports) > 0) {
				$backend_url = $backend->getBackendUrl('main');

                                ?>
				<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
				<script type="text/javascript">
					var selectedReport = '';

					function toggleDiv(id)
					{
						var d = document.getElementById(id);
						var disp = "none";

						if (d.style.display == "none") {
							disp = "block";
						}

						d.style.display = disp;
					}


					function regenCallback(responseText)
					{
						document.getElementById(selectedReport).innerHTML = responseText;
						bg = document.getElementById(selectedReport+"_regen").style.background;
						document.getElementById(selectedReport+"_regen").style.background = "#e8ddeb";
						window.setTimeout('document.getElementById(selectedReport+"_regen").style.background = bg;', 1500);
					}


					function regen(report)
					{
						selectedReport = report;
						var regenElement = document.getElementById(report+"_regen");
						var reportElement = document.getElementById(report+"_report");
						reportElement.style.opacity = "0.4";
						reportElement.style.filter = "alpha(opacity=40)";
						regenElement.innerHTML = "Regenerating...";
						JsHttpConnector.submitRequest('<?php echo $backend_url; ?>&backend_section=sys_maintenance&report_type_code='+report, regenCallback);
					}
				</script>

                                <table class="sq-backend-table">
                                        <tr>
                                                <th><?php echo translate('name'); ?></th>
                                                <th><?php echo translate('description'); ?></th>
                                        </tr>
                                <?php

				// Get report name and description and sort reports by name
				$report_list = Array();
                                foreach ($reports as $report_type_code) {
                                        $report_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($report_type_code);
					$report_name = $report_info['name'];
					$report_list[$report_name] = Array(
									'type_code'	=> $report_type_code,
									'description'	=> $report_info['description'],
								     );
				}

				ksort($report_list);
				reset($report_list);

				$expand_reports = '';
				if (isset($_REQUEST['expand'])) $expand_reports = $_REQUEST['expand'];

                                foreach ($report_list as $report_name => $report_info) {
                                        ?>
                                        <tr>
                                                <td class="sq-backend-table-cell" style="background: white;"><a href="#<?php echo $report_info['type_code']; ?>" onclick="toggleDiv('<?php echo $report_info['type_code']; ?>');"><?php echo $report_name; ?></a></td>
                                                <td class="sq-backend-table-cell" style="background: white;"><?php echo $report_info['description']; ?>
						<div id="<?php echo $report_info['type_code']; ?>" style="display: <?php echo (($report_info['type_code'] == 'report_'.$expand_reports)?'block':'none'); ?>">
							<?php

							$this->_paintReport($o, $report_info['type_code']);

							?>
						</div>
						</td>
                                        </tr>
                                        <?php
                                }
                                ?>
                                </table>
                                <?php
                        } else {
				echo translate('general_reports_not_installed');
                        }
                $o->closeField();
                $o->closeSection();

	}//end paintBackend()


	/**
	* Paints the interface of a report
	*
	* @param object	&$o			Reference to Outputter object
	* @param string	$report_type_code	Asset type code of the report
	*
	* @return void
	* @access private
	*/
	private function _paintReport(&$o, $report_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($report_type_code)) {
			$am->includeAsset($report_type_code);
			?>
			<br>
			<div id="<?php echo $report_type_code; ?>_report">
			<?php
				call_user_func_array(Array($report_type_code, 'paintReport'), Array($o, $report_type_code));
			?>
			</div>
			<br><br>
			<?php
		}

	}//end _paintReport()


	/**
	* Processes the interface of a report
	*
	* @param object	&$o			Reference to Outputter object
	* @param string	$report_type_code	Asset type code of the report
	*
	* @return void
	* @access private
	*/
	private function _processReport(&$o, $report_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($report_type_code)) {
			$am->includeAsset($report_type_code);
			call_user_func_array(Array($report_type_code, 'processReport'), Array($o, $report_type_code));
		}

	}//end _processReport()


}//end class

?>
