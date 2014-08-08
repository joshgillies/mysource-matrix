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
*
* $Id: hipo_reminder.inc,v 1.2 2012/08/30 01:09:19 ewang Exp $
*
*/

/**
* Warranty Expiry Reminder Message
* Static message used when there is a warranty key provided by Squiz - but it
* has expired
*
* Purpose
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
?>
<!DOCTYPE html>
<!--[if lt IE 8 ]> 	<html class="ie hipo">  					<![endif]-->
<!--[if IE 8 ]> 	<html class="ie ie8 hipo">  				<![endif]-->
<!--[if IE 9 ]> 	<html class="ie ie9 gtie8 hipo">  			<![endif]-->
<!--[if IE 10 ]> 	<html class="ie ie-10 gtie8 gtie9 hipo"> 	<![endif]-->
<!--[if !IE]><!--> 	<html class="hipo">      					<!--<![endif]-->
	<head>
		<meta charset="utf-8"/>
		<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"/>
		<title><?php echo SQ_SYSTEM_LONG_NAME ?> - Stalled hipo process </title>
		<link rel="stylesheet" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" type="text/css" />
	</head>
	<body class="sq sq-hipo-body">
		<div class="sq-standalone-wrapper sq-hipo-reminder">

			<table class="sq-standalone-table">
				<tbody>
					<tr class="sq-standalone-header">
						<td colspan="2">
							<img src="<?php echo sq_web_path('lib').'/web/images/icons/hipo.png'; ?>" alt="Powered by Highly Intensive Processing Object (HIPO) Technology - Version 5" /> <strong>Stalled HIPO Processes</strong>
						</td>
					</tr>
					<tr class="sq-standalone-body">
						<td colspan="2">
							<div class="sq-standalone-content">
								<div>
									<div class="sq-warning-message">
										<p><strong>Warning:</strong> You have stalled HIPO jobs.</p>
									</div>
									<table>
										<tr>
											<td class="sq-backend-section-subheading">Current HIPO Jobs</td>
										</tr>
									</table>	
									<table class="sq-backend-table">
										<tbody>
											<tr>
												<th class="sq-backend-table-header">Job Type</th>
												<th class="sq-backend-table-header">Last Updated</th>
												<th class="sq-backend-table-header" style="text-align: center;">Percent Complete</th>
												<th class="sq-backend-table-header">Code Name</th>
											</tr>
											<?php
												require_once SQ_FUDGE_PATH.'/general/datetime.inc';
												$now = time();
												foreach ($stalled_hipo_array as $job_index => $job_info) {
													?>
													<tr>
														<td class="sq-backend-table-cell"><?php echo str_replace('_', ' ', $job_info['job_type']); ?></td>
														<td class="sq-backend-table-cell"><?php echo easy_time_total(strtotime($job_info['last_updated']) - $now, TRUE); ?></td>
														<td class="sq-backend-table-cell" style="text-align: center;"><?php echo $job_info['percent_done'].'%'; ?></td>
														<td class="sq-backend-table-cell"><?php echo $job_info['code_name']; ?></td>
													</tr>
													<?php
												}
											?>
										</tbody>
									</table>
									<p>Click the <strong>HIPO Herder</strong> button below to proceed to your HIPO Herder or click <strong>Logout</strong> button to exit the system.</p>
								</div>
							</div>
						</td>
					</tr>
					<tr class="sq-standalone-footer">
						<td style="text-align: left">
							<form method="post" action="?backend_section=hipo_herder">
								<input type="submit" value="HIPO Herder" class="sq-hipo-button-enabled">
							</form>
						</td>
						<td style="text-align: right">
							<form method="post" action="?SQ_ACTION=real_logout">
								<input type="submit" value="Logout" class="sq-btn-blue sq-hipo-button-enabled next">
							</form>
						</td>					
					</tr>
				</tbody>
			</table>
		</div><!--.sq-standalone-wrapper-->
	</body>
</html>
