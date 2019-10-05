#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import smtplib
import sys
import tarfile
from datetime import datetime
from email import encoders
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.utils import formatdate
from os.path import basename
from pathlib import Path


def send_email(sender: str, to: str, subject: str, text: str, login: str, password: str, smtp_host: str, files=None):
    msg = MIMEMultipart()
    msg['From'] = sender
    msg['To'] = to
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = subject

    msg.attach(MIMEText(text))

    for filename in files or []:
        with open(filename, 'rb') as file:
            part = MIMEBase('application', 'x-gzip')
            part.set_payload(file.read())
            encoders.encode_base64(part)
        part.add_header('Content-Disposition', 'attachment', filename=f'{basename(filename)}')
        part.add_header('Content-Type', 'application/x-gzip', filename=f'{basename(filename)}')
        msg.attach(part)

    with smtplib.SMTP_SSL(smtp_host) as server:
        # server.set_debuglevel(1)
        server.ehlo_or_helo_if_needed()
        server.login(login, password)
        server.sendmail(sender, to, msg.as_string())


def create_tarfile(source_dir: Path, output_filename):
    with tarfile.open(output_filename, "w:gz") as tar:
        tar.add(source_dir, arcname=source_dir.name)


def main():
    # TODO add parseargs
    assert sys.version_info >= (3, 6), ("Python version should be >= 3.6. "
                                        "To check your current python version run: 'python3 -V'.")
    basedir: Path = Path(__file__).parent.resolve()
    task_dir = basedir / os.environ['DC_MAIL_PA']
    tar_filename = basedir / f'{task_dir.name}.tar.gz'
    create_tarfile(source_dir=task_dir, output_filename=tar_filename)

    sender = os.environ['DC_MAIL_SENDER']
    send_email(
        sender=sender,
        to=os.environ['DC_MAIL_TO'],
        subject=os.environ['DC_MAIL_SUBJECT'],
        text=os.environ['DC_MAIL_TEXT'],
        login=sender,
        password=os.environ['DC_MAIL_PASS'],
        smtp_host=os.environ['DC_MAIL_HOST'],
        files=[tar_filename]
    )
    print(f'Sent successfully at {datetime.now().strftime("%d-%m-%Y %H:%M:%S")}.')


if __name__ == '__main__':
    main()
