using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

class CircularBuffer<T>  //<T> = tipo de dado
{
    T[] _buffer;
    private int _head; //fim do buffer
    private int _tail; //começo do buffer
    private int _length; //tamanho do buffer
    private int _bufferSize; //capacidade do buffer
    object _lock = new object(); // para threads multiplas sincronizando o enqueuing/dequeuing

    //construtor
    public CircularBuffer(int bufferSize)
    {
        _buffer = new T[bufferSize]; //cria um novo buffer
        _bufferSize = bufferSize; //atualiza a capacidade do buffer
        _head = _bufferSize - 1; //indica onde está o final do buffer circular
    }

    //propriedades

    //verificar se está vazio
    public bool IsEmpty
    {
        get { return _length == 0; } //só está vazio se o tamanho do buffer é 0
    }

    //verificar se já está cheio
    public bool IsFull
    {
        get { return _length == _bufferSize; }// só está cheio se o tamanho do buffer é igual à capacidade
    }


    //proxima posição
    private int NextPosition(int pos)
    {
        return (pos + 1) % _bufferSize; //será o resto da divisão da posição atual pela capacidade do buffer
    }


    //Dequeuing
    public T Dequeue()
    {
        lock (_lock)
        {
            //se o buffer está vazio
            if (IsEmpty)
            {
                throw new InvalidOperationException("Fila Vazia"); //erro
            }
            T dequeued = _buffer[_tail]; //elimina o primeiro elemento do buffer
            _tail = NextPosition(_tail); //atualiza o ponteiro inicial do buffer
            _length--; //decrementa o tamanho
            return dequeued; //retorna o buffer
        }
    }

    //Queuing
    public void Enqueue(T add)
    {
        lock (_lock)
        {
            _head = NextPosition(_head); //atualiza a posiçao
            _buffer[_head] = add; //adiciona na posição
            //se o buffer está cheio
            if (IsFull)
            {
                _tail = NextPosition(_tail); //sobrescreve a primeira posição
                Console.WriteLine("Perda de dados...");
            }
            else
                _length++;//aumenta o tamanho
        }

    }

}
