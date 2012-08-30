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
<html>
	<head>
		<title><?php echo SQ_SYSTEM_LONG_NAME ?> - Stalled hipo process </title>
		<link rel="stylesheet" href="<?php echo sq_web_path('lib').'/web/css/hipo_reminder.css' ?>" type="text/css" />

		<!--[if IE]>
			<style type="text/css">
				.hipo-reminder-body {
					width:				100%;
					border-bottom:		5px solid #f1f1f1;
					padding:			0 10px 10px;
				}
				.hipo-reminder-warning {
					margin-top:			16px;
				}

			</style>
		<![endif]-->

	</head>
	<body>
		<div class="hipo-reminder">
			<h1><?php echo sq_get_icon(sq_web_path('lib').'/web/images/icons/header/system_config.png', 20, 20, 'Key icon', '', 'align="absmiddle"') ?>  Stalled HIPO Process </h1>
			<div class="hipo-reminder-body">

				<p class="hipo-reminder-warning">YOU HAVE STALLED HIPO JOBS</p>

				<table class="sq-backend-section-table">
					<tbody>
						<tr class="sq-backend-section-heading-container">
							<td>
								<table border="0" cellpadding="0" cellspacing="0">
									<tbody>
										<tr>
											<td class="sq-backend-section-heading">Current HIPO Jobs</td>
											<td><?php echo sq_get_icon(sq_web_path('lib').'/web/images/section_icon.gif', 27, 21, 'Section icon', '', 'align="absmiddle"') ?></td>
										</tr>
									</tbody>
								</table>
							</td>
						</tr>
						<tr>
							<td>
								<table class="sq-backend-section-table-inner" border="0" cellpadding="0" cellspacing="0">
									<tbody>
										<tr>
											<td class="sq-backend-field sq-backend-field-newline" colspan="2">
												&nbsp;
											</td>
										</tr>
										<tr>
											<td class="sq-backend-data sq-backend-data-newline" colspan="2">
												<table class="sq-backend-table">
													<tbody>
														<tr>
															<td class="sq-backend-table-header">Job Type</td>
															<td class="sq-backend-table-header">Last Updated</td>
															<td class="sq-backend-table-header" style="text-align: center;">Percent Complete</td>
															<td class="sq-backend-table-header">Code Name</td>
														</tr>
														<tr>
														<?php
															require_once SQ_FUDGE_PATH.'/general/datetime.inc';
															$now = time();
															foreach ($stalled_hipo_array as $job_index => $job_info) {
																?>
																	<td class="sq-backend-table-cell" style="text-align: left;"><?php echo $job_info['job_type']; ?></td>
																	<td class="sq-backend-table-cell" style="text-align: left;"><?php echo easy_time_total(strtotime($job_info['last_updated']) - $now, TRUE); ?></td>
																	<td class="sq-backend-table-cell" style="text-align: center;"><?php echo $job_info['percent_done'].'%'; ?></td>
																	<td class="sq-backend-table-cell" style="text-align: left;"><?php echo $job_info['code_name']; ?></td>
																</tr><tr><?php
															}
															?>
														</tr>
													</tbody>
												</table>
											</td>
										</tr>
									</tbody>
								</table>
							</td>
						</tr>
					</tbody>
				</table>

				<h2>Continue to HIPO Jobs Screen</h3>

				<p>Click the <strong>HIPO Herder</strong> button below to proceed to your HIPO Herder or click Logout to exit the system.</p>


				<table width="100%" border="0">
					<tbody><tr>
						<td>
							<form method="post" action="?backend_section=hipo_herder">
								<input type="submit" value="HIPO Herder">
							</form>
						</td>
						<td align = "right">
							<form method="post" action="?SQ_ACTION=real_logout">
								<input type="submit" value="Logout">
							</form>
						</td>
					</tr>
					</tbody>
				</table>

			</div>

			<div class="hipo-reminder-footer">
				<a href="http://matrix.squiz.net/" target="_blank"><?php echo SQ_SYSTEM_LONG_NAME ?></a>
			</div>
		</div>

	</body>
</html>
