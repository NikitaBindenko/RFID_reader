import asyncio

from main import  bot ,dp, list_of_users

from aiogram.types import Message
from config import admin_id
import serial

@dp.message_handler(commands=['start'])
async def save_id(message):
     await bot.send_message(chat_id=message.chat.id, text="Бот запущен!")
     if not (message.chat.id) in list_of_users:
         file=open("D:/Python/telegrambot/users.txt", "a")
         file.write(str(message.chat.id) + '\n')
         list_of_users.add(message.chat.id)


@dp.message_handler(commands=['watch'])
async def send_info(message):
    ser = serial.Serial('COM3', 9600)
    while (True):
        line = ser.readline()
        line = line.decode('utf-8')
        print(line)
        for user in list_of_users:
            await asyncio.sleep(1/100)
            await bot.send_message(user, text=line)

